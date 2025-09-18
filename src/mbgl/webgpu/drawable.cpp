#include <mbgl/webgpu/drawable.hpp>

#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/drawable_impl.hpp>
#include <sstream>
#include <mbgl/webgpu/index_buffer_resource.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/variant.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/monotonic_timer.hpp>

#include <cassert>
#include <memory>
#if !defined(NDEBUG)
#include <sstream>
#endif

namespace mbgl {
namespace webgpu {

namespace {
#if !defined(NDEBUG)
std::string debugLabel(const Drawable& drawable) {
    std::ostringstream ss;
    ss << drawable.getName() << "#" << drawable.getID();
    return ss.str();
}
#endif

struct alignas(16) GlobalUBOIndexData {
    uint32_t value = 0;
    uint32_t pad[3] = {0, 0, 0};
};

WGPUVertexFormat wgpuVertexFormatOf(gfx::AttributeDataType type) {
    switch (type) {
        case gfx::AttributeDataType::Byte:
            return WGPUVertexFormat_Sint8;
        case gfx::AttributeDataType::Byte2:
            return WGPUVertexFormat_Sint8x2;
        case gfx::AttributeDataType::Byte4:
            return WGPUVertexFormat_Sint8x4;
        case gfx::AttributeDataType::UByte:
            return WGPUVertexFormat_Uint8;
        case gfx::AttributeDataType::UByte2:
            return WGPUVertexFormat_Uint8x2;
        case gfx::AttributeDataType::UByte4:
            return WGPUVertexFormat_Uint8x4;
        case gfx::AttributeDataType::Short:
            return WGPUVertexFormat_Sint16;
        case gfx::AttributeDataType::Short2:
            return WGPUVertexFormat_Sint16x2;
        case gfx::AttributeDataType::Short4:
            return WGPUVertexFormat_Sint16x4;
        case gfx::AttributeDataType::UShort:
            return WGPUVertexFormat_Uint16;
        case gfx::AttributeDataType::UShort2:
            return WGPUVertexFormat_Uint16x2;
        case gfx::AttributeDataType::UShort4:
            return WGPUVertexFormat_Uint16x4;
        case gfx::AttributeDataType::Int:
            return WGPUVertexFormat_Sint32;
        case gfx::AttributeDataType::Int2:
            return WGPUVertexFormat_Sint32x2;
        case gfx::AttributeDataType::Int3:
            return WGPUVertexFormat_Sint32x3;
        case gfx::AttributeDataType::Int4:
            return WGPUVertexFormat_Sint32x4;
        case gfx::AttributeDataType::UInt:
            return WGPUVertexFormat_Uint32;
        case gfx::AttributeDataType::UInt2:
            return WGPUVertexFormat_Uint32x2;
        case gfx::AttributeDataType::UInt3:
            return WGPUVertexFormat_Uint32x3;
        case gfx::AttributeDataType::UInt4:
            return WGPUVertexFormat_Uint32x4;
        case gfx::AttributeDataType::Float:
            return WGPUVertexFormat_Float32;
        case gfx::AttributeDataType::Float2:
            return WGPUVertexFormat_Float32x2;
        case gfx::AttributeDataType::Float3:
            return WGPUVertexFormat_Float32x3;
        case gfx::AttributeDataType::Float4:
            return WGPUVertexFormat_Float32x4;
        default:
            assert(!"Unsupported vertex attribute format");
            return WGPUVertexFormat_Float32x2; // Default fallback
    }
}
} // namespace

// Simple wrapper to adapt gfx::IndexBuffer to IndexBufferBase
struct IndexBuffer : public gfx::IndexBufferBase {
    IndexBuffer(std::unique_ptr<gfx::IndexBuffer>&& buffer_)
        : buffer(std::move(buffer_)) {}
    ~IndexBuffer() override = default;

    std::unique_ptr<mbgl::gfx::IndexBuffer> buffer;
};

Drawable::Drawable(std::string name_)
    : gfx::Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

Drawable::~Drawable() {
    // Clean up WebGPU resources safely
    // Note: We don't own the pipeline - it's owned by the shader program
    // So we should NOT release it here
    impl->pipelineState = nullptr;

    for (auto bindGroup : impl->bindGroups) {
        if (bindGroup) {
            wgpuBindGroupRelease(bindGroup);
        }
    }
    impl->bindGroups.clear();
    impl->bindGroupIndices.clear();

    // Uniform buffers are managed by UniformBufferArray
    // Vertex and index buffers are now managed through attributeBindings and indexes

}

namespace {

[[maybe_unused]] WGPUPrimitiveTopology getPrimitiveTopology(const gfx::DrawModeType type) noexcept {
    switch (type) {
        default:
            assert(false);
            [[fallthrough]];
        case gfx::DrawModeType::Points:
            return WGPUPrimitiveTopology_PointList;
        case gfx::DrawModeType::Lines:
            return WGPUPrimitiveTopology_LineList;
        case gfx::DrawModeType::LineStrip:
            return WGPUPrimitiveTopology_LineStrip;
        case gfx::DrawModeType::Triangles:
            return WGPUPrimitiveTopology_TriangleList;
    }
}

#if !defined(NDEBUG)
[[maybe_unused]] std::string debugLabel(const gfx::Drawable& drawable) {
    std::ostringstream oss;
    oss << drawable.getID().id() << "/" << drawable.getName() << "/tile=";
    if (const auto& tileID = drawable.getTileID()) {
        oss << util::toString(*tileID);
    } else {
        oss << "(null)";
    }
    return oss.str();
}
#endif // !defined(NDEBUG)

} // namespace

void Drawable::setColorMode(const gfx::ColorMode&) {
    // Color mode is handled during pipeline creation
}

void Drawable::setEnableStencil(bool value) {
    if (getEnableStencil() == value) {
        return;
    }
    gfx::Drawable::setEnableStencil(value);
    impl->pipelineState = nullptr;  // Reset pipeline to force recreation with new stencil state
}

void Drawable::setEnableDepth(bool value) {
    if (getEnableDepth() == value) {
        return;
    }
    gfx::Drawable::setEnableDepth(value);
    impl->pipelineState = nullptr;  // Reset pipeline to force recreation with new depth state
}

void Drawable::setSubLayerIndex(int32_t value) {
    if (getSubLayerIndex() == value) {
        return;
    }
    gfx::Drawable::setSubLayerIndex(value);
}

void Drawable::setDepthType(gfx::DepthMaskType value) {
    if (getDepthType() == value) {
        return;
    }
    gfx::Drawable::setDepthType(value);
    impl->pipelineState = nullptr;  // Reset pipeline to force recreation with new depth mask
}

void Drawable::setShader(gfx::ShaderProgramBasePtr value) {
    if (shader == value) {
        return;
    }
    shader = std::move(value);
    impl->pipelineState = nullptr;  // Reset pipeline when shader changes
}

void Drawable::upload(gfx::UploadPass& uploadPass) {
    // std::stringstream uploadCallMsg;
    // uploadCallMsg << "WebGPU Drawable::upload called for " << getName();
    // Log::Info(Event::Render, uploadCallMsg.str());

    if (isCustom) {
        return;
    }
    if (!shader) {
        Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
        assert(false);
        return;
    }

    auto& webgpuUploadPass = static_cast<webgpu::UploadPass&>(uploadPass);
    auto& gfxContext = webgpuUploadPass.getContext();
    [[maybe_unused]] auto& context = static_cast<webgpu::Context&>(gfxContext);
    constexpr auto usage = gfx::BufferUsageType::StaticDraw;

    // We need either raw index data or a buffer already created from them.
    // We can have a buffer and no indexes, but only if it's not marked dirty.
    if (!impl->indexes || (impl->indexes->empty() && (!impl->indexes->getBuffer() || impl->indexes->getDirty()))) {
        assert(!"Missing index data");
        return;
    }

    if (!impl->indexes->getBuffer() || impl->indexes->getDirty()) {
        // Create or update a buffer for the index data.  We don't update any
        // existing buffer because it may still be in use by the previous frame.
        auto indexBufferResource = webgpuUploadPass.createIndexBufferResource(
            impl->indexes->data(),
            impl->indexes->bytes(),
            usage,
            /*persistent=*/false);
        auto gfxIndexBuffer = std::make_unique<gfx::IndexBuffer>(
            impl->indexes->elements(),
            std::move(indexBufferResource));
        auto buffer = std::make_unique<IndexBuffer>(std::move(gfxIndexBuffer));

        impl->indexes->setBuffer(std::move(buffer));
        impl->indexes->setDirty(false);
    }

    // Upload vertex attributes (like Metal does)
    const bool buildAttribs = !vertexAttributes || !attributeUpdateTime ||
                              vertexAttributes->isModifiedAfter(*attributeUpdateTime);

    // std::stringstream uploadMsg;
    // uploadMsg << "WebGPU upload() buildAttribs=" << (buildAttribs ? "true" : "false")
    //           << ", attributeUpdateTime=" << (attributeUpdateTime ? "set" : "null");
    // Log::Info(Event::Render, uploadMsg.str());

    if (buildAttribs) {
#if !defined(NDEBUG)
        const auto debugGroup = webgpuUploadPass.createDebugGroup(debugLabel(*this));
#endif

        if (!vertexAttributes) {
            vertexAttributes = std::make_shared<gfx::VertexAttributeArray>();
        }

        // Apply drawable values to shader defaults (matching Metal's approach)
        std::vector<std::unique_ptr<gfx::VertexBufferResource>> vertexBuffers;
        auto attributeBindings_ = webgpuUploadPass.buildAttributeBindings(impl->vertexCount,
                                                                           impl->vertexType,
                                                                           /*vertexAttributeIndex=*/-1,
                                                                           /*vertexData=*/{},
                                                                           shader->getVertexAttributes(),
                                                                           *vertexAttributes,
                                                                           usage,
                                                                           attributeUpdateTime,
                                                                           vertexBuffers);


        if (impl->attributeBindings != attributeBindings_) {
            impl->attributeBindings = std::move(attributeBindings_);
            impl->vertexDescHash = 0; // Reset hash when bindings change
            impl->pipelineState = nullptr; // Reset pipeline state when attribute bindings change
            // Store the dummy vertex buffers to keep them alive
            impl->dummyVertexBuffers = std::move(vertexBuffers);
        }
    }

    // Build instance buffer (like Metal does)
    const bool buildInstanceBuffer =
        (instanceAttributes && (!attributeUpdateTime || instanceAttributes->isModifiedAfter(*attributeUpdateTime)));

    if (buildInstanceBuffer) {
        // Apply instance values to shader defaults (matching Metal's approach)
        std::vector<std::unique_ptr<gfx::VertexBufferResource>> instanceBuffers;
        auto instanceBindings_ = webgpuUploadPass.buildAttributeBindings(impl->vertexCount,
                                                                         impl->vertexType,
                                                                         /*vertexAttributeIndex=*/-1,
                                                                         /*vertexData=*/{},
                                                                         shader->getInstanceAttributes(),
                                                                         *instanceAttributes,
                                                                         usage,
                                                                         attributeUpdateTime,
                                                                         instanceBuffers);


        if (impl->instanceBindings != instanceBindings_) {
            impl->instanceBindings = std::move(instanceBindings_);
            impl->pipelineState = nullptr; // Reset pipeline state when instance bindings change
            // Store the dummy instance buffers to keep them alive
            if (!instanceBuffers.empty()) {
                impl->dummyVertexBuffers.insert(impl->dummyVertexBuffers.end(),
                                               std::make_move_iterator(instanceBuffers.begin()),
                                               std::make_move_iterator(instanceBuffers.end()));
            }
        }
    }

    // Upload uniform buffers to ensure they're ready for the GPU
    // This is critical for making drawables visible
    for (size_t i = 0; i < impl->uniformBuffers.allocatedSize(); ++i) {
        auto& uniformBuffer = impl->uniformBuffers.get(i);
        if (uniformBuffer) {
            // Make sure the uniform buffer has its data uploaded
            // The uniform buffer should handle its own upload internally
            // but we need to ensure it's marked as not dirty
            auto* webgpuUniformBuffer = static_cast<webgpu::UniformBuffer*>(uniformBuffer.get());
            if (webgpuUniformBuffer) {
                // The uniform buffer's data should already be uploaded when it was created/updated
                // Just ensure we have a valid buffer handle
                if (!webgpuUniformBuffer->getBuffer()) {
                    Log::Error(Event::Render, "Uniform buffer " + std::to_string(i) + " has no GPU buffer!");
                }
            }
        }
    }

    // Upload textures if needed
    const bool texturesNeedUpload = std::any_of(
        textures.begin(), textures.end(), [](const auto& texture) { return texture && texture->needsUpload(); });

    if (texturesNeedUpload) {
        for (auto& texture : textures) {
            if (texture && texture->needsUpload()) {
                texture->upload();
            }
        }
    }

    attributeUpdateTime = util::MonotonicTimer::now();
}

void Drawable::draw(PaintParameters& parameters) const {
    static int drawCallCount = 0;
    drawCallCount++;
    if (drawCallCount <= 2000) {
        Log::Info(Event::Render, "WebGPU Drawable::draw #" + std::to_string(drawCallCount) + " for " + getName() +
                  " pass=" + std::to_string(mbgl::underlying_type(parameters.pass)) +
                  " shader=" + (shader ? shader->typeName().data() : "null"));
    }

    if (isCustom) {
        return;
    }

    // Get WebGPU context and render pass (following Metal's pattern)
    auto& context = static_cast<webgpu::Context&>(parameters.context);
    auto& webgpuRenderPass = static_cast<webgpu::RenderPass&>(*parameters.renderPass);
    WGPURenderPassEncoder renderPassEncoder = webgpuRenderPass.getEncoder();
    if (!renderPassEncoder) {
        Log::Error(Event::Render, "No render pass encoder available");
        assert(false);
        return;
    }

    if (!shader) {
        Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
        assert(false);
        return;
    }

    if (!getEnabled()) {
        return;
    }

    // Get WebGPU backend and device
    auto& backend = static_cast<webgpu::RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());

    if (!device || !queue) {
        return;
    }

#if !defined(NDEBUG)
    // Debug group creation would go here (Metal has this but we need to implement it for WebGPU)
    // const auto debugGroup = parameters.encoder->createDebugGroup(getName());
#endif

    // Handle uboIndex like Metal does
    // In WebGPU, we'll need to handle this differently since we use bind groups
    // Metal uses setVertexBytes/setFragmentBytes for the uboIndex
    // For now, we'll store it for use in bind group creation

    // Bind uniform buffers (like Metal does)
    impl->uniformBuffers.bind(webgpuRenderPass);

    // Check index buffer is valid (like Metal does)
    if (impl->indexes && (!impl->indexes->getBuffer() || impl->indexes->getDirty())) {
        assert(!"Index buffer not uploaded");
        return;
    }

    // Build bind groups based on shader metadata
    if (shader) {
        auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
        if (webgpuShader) {
            WGPUDevice deviceHandle = static_cast<WGPUDevice>(backend.getDevice());

            for (auto bindGroup : impl->bindGroups) {
                if (bindGroup) {
                    wgpuBindGroupRelease(bindGroup);
                }
            }
            impl->bindGroups.clear();
            impl->bindGroupIndices.clear();

            if (deviceHandle) {
                const auto& groupOrder = webgpuShader->getBindGroupOrder();
                const auto* globalBuffers = webgpuRenderPass.getGlobalUniformBuffers();

                const auto findTextureForBinding = [&](uint32_t binding, bool samplerBinding) -> Texture2D* {
                    for (size_t texIndex = 0; texIndex < textures.size(); ++texIndex) {
                        if (!textures[texIndex]) {
                            continue;
                        }
                        if (const auto location = webgpuShader->getSamplerLocation(texIndex)) {
                            if ((samplerBinding && *location == binding) || (!samplerBinding && (*location == binding || *location + 1 == binding))) {
                                return static_cast<Texture2D*>(textures[texIndex].get());
                            }
                        }
                    }
                    return nullptr;
                };

                for (size_t slot = 0; slot < groupOrder.size(); ++slot) {
                    const uint32_t group = groupOrder[slot];
                    auto layout = webgpuShader->getBindGroupLayout(group);
                    if (!layout) {
                        impl->bindGroups.push_back(nullptr);
                        impl->bindGroupIndices.push_back(group);
                        continue;
                    }

                    const auto& bindingInfos = webgpuShader->getBindingInfosForGroup(group);
                    std::vector<WGPUBindGroupEntry> entries;
                    entries.reserve(bindingInfos.size());
                    bool validBindings = true;

                    for (const auto& bindingInfo : bindingInfos) {
                        WGPUBindGroupEntry entry = {};
                        entry.binding = bindingInfo.binding;

                        switch (bindingInfo.type) {
                            case ShaderProgram::BindingType::UniformBuffer:
                            case ShaderProgram::BindingType::ReadOnlyStorageBuffer:
                            case ShaderProgram::BindingType::StorageBuffer: {
                                std::shared_ptr<gfx::UniformBuffer> buffer = impl->uniformBuffers.get(bindingInfo.binding);
                                if (!buffer && globalBuffers) {
                                    buffer = globalBuffers->get(bindingInfo.binding);
                                }
                                if (!buffer && bindingInfo.binding == shaders::idGlobalUBOIndex) {
                                    GlobalUBOIndexData indexData;
                                    indexData.value = getUBOIndex();
                                    context.emplaceOrUpdateUniformBuffer(impl->uboIndexUniform,
                                                                         &indexData,
                                                                         sizeof(indexData),
                                                                         false);
                                    if (impl->uboIndexUniform) {
                                        impl->uniformBuffers.set(bindingInfo.binding, impl->uboIndexUniform);
                                        buffer = impl->uboIndexUniform;
                                    }
                                }

                                if (!buffer) {
                                    if (drawCallCount <= 200) {
                                        Log::Warning(Event::Render,
                                                     "No buffer found for binding " + std::to_string(bindingInfo.binding) +
                                                         " type=" + std::to_string(static_cast<int>(bindingInfo.type)) +
                                                         " group=" + std::to_string(group));
                                    }
                                    validBindings = false;
                                    break;
                                }

                                const auto* webgpuUniform = static_cast<const webgpu::UniformBuffer*>(buffer.get());
                                if (!webgpuUniform || !webgpuUniform->getBuffer()) {
                                    if (drawCallCount <= 200) {
                                        Log::Warning(Event::Render,
                                                     "Uniform buffer missing GPU handle for binding " +
                                                         std::to_string(bindingInfo.binding) + " group=" +
                                                         std::to_string(group));
                                    }
                                    validBindings = false;
                                    break;
                                }

                                entry.buffer = webgpuUniform->getBuffer();
                                entry.offset = 0;
                                entry.size = buffer->getSize();
                                break;
                            }
                            case ShaderProgram::BindingType::Sampler: {
                                Texture2D* texture = findTextureForBinding(bindingInfo.binding, true);
                                if (!texture) {
                                    validBindings = false;
                                    break;
                                }
                                if (!texture->getSampler()) {
                                    texture->setSamplerConfiguration(texture->getSamplerState());
                                }
                                entry.sampler = texture->getSampler();
                                break;
                            }
                            case ShaderProgram::BindingType::Texture: {
                                Texture2D* texture = findTextureForBinding(bindingInfo.binding, false);
                                if (!texture) {
                                    validBindings = false;
                                    break;
                                }
                                if (!texture->getTexture() || !texture->getTextureView()) {
                                    texture->create();
                                }
                                if (!texture->getTextureView()) {
                                    validBindings = false;
                                    break;
                                }
                                entry.textureView = texture->getTextureView();
                                break;
                            }
                        }

                        if (!validBindings) {
                            break;
                        }

                        entries.push_back(entry);
                    }

                    if (!validBindings || entries.empty()) {
                        impl->bindGroups.push_back(nullptr);
                        impl->bindGroupIndices.push_back(group);
                        continue;
                    }

                    const std::string label = getName() + " bind-group " + std::to_string(group);
                    WGPUStringView labelView = {label.c_str(), label.length()};

                    WGPUBindGroupDescriptor descriptor = {};
                    descriptor.label = labelView;
                    descriptor.layout = layout;
                    descriptor.entryCount = entries.size();
                    descriptor.entries = entries.data();

                    WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(deviceHandle, &descriptor);
                    impl->bindGroups.push_back(bindGroup);
                    impl->bindGroupIndices.push_back(group);
                }
            }
        }
    }

    // Get pipeline from shader if we don't have it yet (like Metal)
    auto& shaderWebGPU = static_cast<mbgl::webgpu::ShaderProgram&>(*shader);

    if (!impl->pipelineState) {
        // Create vertex layout from attribute bindings
        // IMPORTANT: These must be kept alive until pipeline creation is done
        std::vector<std::vector<WGPUVertexAttribute>> vertexAttrs;
        std::vector<WGPUVertexBufferLayout> vertexLayouts;

        // Create vertex buffer layouts from attribute bindings
        for (size_t i = 0; i < impl->attributeBindings.size(); ++i) {
            const auto& binding = impl->attributeBindings[i];
            if (!binding.has_value() || !binding->vertexBufferResource) {
                continue;
            }

            // Create vertex attribute for this buffer
            vertexAttrs.emplace_back();
            auto& attrs = vertexAttrs.back();

            WGPUVertexAttribute attr = {};
            attr.format = wgpuVertexFormatOf(binding->attribute.dataType);
            attr.offset = binding->attribute.offset;
            attr.shaderLocation = static_cast<uint32_t>(i);  // Use the actual attribute index
            attrs.push_back(attr);
        }

        // Now create the layouts with stable pointers to attributes
        size_t attrIndex = 0;
        for (const auto& binding : impl->attributeBindings) {
            if (!binding.has_value() || !binding->vertexBufferResource) {
                continue;
            }

            WGPUVertexBufferLayout layout = {};
            layout.arrayStride = binding->vertexStride;
            layout.stepMode = WGPUVertexStepMode_Vertex;
            layout.attributeCount = vertexAttrs[attrIndex].size();
            layout.attributes = vertexAttrs[attrIndex].data();
            vertexLayouts.push_back(layout);

            attrIndex++;
        }

        // Get render pipeline similar to Metal's getRenderPipelineState
        // Use the actual color mode from the drawable (like Metal does)
        const auto& colorMode = getColorMode();

        // Get the renderable from render pass descriptor like Metal does
        const auto& renderable = webgpuRenderPass.getDescriptor().renderable;

        impl->pipelineState = shaderWebGPU.getRenderPipeline(
            renderable,
            vertexLayouts.empty() ? nullptr : vertexLayouts.data(),
            vertexLayouts.size(),
            colorMode,
            std::nullopt);
        if (!impl->pipelineState) {
            Log::Error(Event::Render, "getRenderPipeline returned null");
        } else {
            // Log::Info(Event::Render, "Pipeline created successfully: " + std::to_string(reinterpret_cast<uintptr_t>(impl->pipelineState)));
        }
    }

    if (impl->pipelineState) {
        if (!renderPassEncoder) {
            Log::Error(Event::Render, "renderPassEncoder is null!");
            return;
        }
        if (drawCallCount <= 200) {
            Log::Info(Event::Render, "Setting pipeline state: encoder=" +
                std::to_string(reinterpret_cast<uintptr_t>(renderPassEncoder)) +
                ", pipeline=" + std::to_string(reinterpret_cast<uintptr_t>(impl->pipelineState)));
        }
        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, impl->pipelineState);
    } else {
        Log::Error(Event::Render, "Failed to create render pipeline state");
        assert(!"Failed to create render pipeline state");
        return;
    }


    // Bind vertex buffers from attributeBindings
    if (drawCallCount <= 200 && getName().find("fill") != std::string::npos) {
        Log::Info(Event::Render, "Total attribute bindings: " + std::to_string(impl->attributeBindings.size()));
        // Debug: log all attribute bindings
        for (size_t i = 0; i < impl->attributeBindings.size(); ++i) {
            const auto& binding = impl->attributeBindings[i];
            if (binding.has_value()) {
                Log::Info(Event::Render, "  AttributeBinding[" + std::to_string(i) + "]: has_value=true, vertexBufferResource=" +
                         (binding->vertexBufferResource ? "present" : "null") +
                         ", dataType=" + std::to_string(static_cast<int>(binding->attribute.dataType)));
            } else {
                Log::Info(Event::Render, "  AttributeBinding[" + std::to_string(i) + "]: has_value=false");
            }
        }
    }

    uint32_t boundBufferCount = 0;
    uint32_t bufferSlot = 0;
    for (size_t i = 0; i < impl->attributeBindings.size(); ++i) {
        const auto& binding = impl->attributeBindings[i];
        if (binding.has_value() && binding->vertexBufferResource) {
            const auto* vertexBufferRes = static_cast<const VertexBufferResource*>(binding->vertexBufferResource);
            if (vertexBufferRes) {
                const auto& buffer = vertexBufferRes->getBuffer();
                if (buffer.getBuffer()) {
                    const uint64_t byteOffset = static_cast<uint64_t>(binding->vertexOffset) * binding->vertexStride;
                    const uint64_t bufferSize = buffer.getSizeInBytes();
                    if (byteOffset >= bufferSize) {
                        Log::Warning(Event::Render,
                                     "Skipping vertex buffer slot " + std::to_string(bufferSlot) +
                                         " for attribute " + std::to_string(i) +
                                         " due to offset " + std::to_string(byteOffset) +
                                         " >= size " + std::to_string(bufferSize));
                        continue;
                    }

                    if (drawCallCount <= 200 && getName().find("fill") != std::string::npos) {
                        Log::Info(Event::Render, "  Binding vertex buffer slot " + std::to_string(bufferSlot) +
                                 " for attribute " + std::to_string(i) +
                                 " size=" + std::to_string(bufferSize) +
                                 " attributeOffset=" + std::to_string(binding->attribute.offset) +
                                 " vertexOffset=" + std::to_string(binding->vertexOffset) +
                                 " byteOffset=" + std::to_string(byteOffset) +
                                 " stride=" + std::to_string(binding->vertexStride) +
                                 " dataType=" + std::to_string(static_cast<int>(binding->attribute.dataType)) +
                                 " bufferPtr=" + std::to_string(reinterpret_cast<uintptr_t>(buffer.getBuffer())));
                    }

                    wgpuRenderPassEncoderSetVertexBuffer(
                        renderPassEncoder,
                        bufferSlot,
                        buffer.getBuffer(),
                        byteOffset,
                        bufferSize - byteOffset);
                    bufferSlot++;
                    boundBufferCount++;
                }
            }
        }
    }

    if (drawCallCount <= 200 && getName().find("fill") != std::string::npos) {
        Log::Info(Event::Render, "Actually bound " + std::to_string(boundBufferCount) + " vertex buffers");
    }

    // Bind index buffer if present
    if (impl->indexes && impl->indexes->elements() > 0 && !impl->indexes->getDirty()) {
        // Get the buffer from indexes
        if (const auto* indexBufferBase = impl->indexes->getBuffer()) {
            if (const auto* webgpuIndexBuffer = static_cast<const IndexBuffer*>(indexBufferBase)) {
                if (webgpuIndexBuffer->buffer) {
                    const auto& indexBufferRes = webgpuIndexBuffer->buffer->getResource<IndexBufferResource>();
                    const auto& bufferResource = indexBufferRes.getBuffer();
                    if (bufferResource.getBuffer()) {
                        // Always use Uint16 format for now
                        WGPUIndexFormat indexFormat = WGPUIndexFormat_Uint16;
                        wgpuRenderPassEncoderSetIndexBuffer(renderPassEncoder,
                                                           bufferResource.getBuffer(),
                                                           indexFormat,
                                                           0,
                                                           bufferResource.getSizeInBytes());
                    }
                }
            }
        }
    }

    // Set bind group
    for (size_t slot = 0; slot < impl->bindGroups.size(); ++slot) {
        const auto bindGroup = impl->bindGroups[slot];
        const auto groupIndex = (slot < impl->bindGroupIndices.size()) ? impl->bindGroupIndices[slot]
                                                                       : static_cast<uint32_t>(slot);
        if (bindGroup) {
            if (drawCallCount <= 200) {
                Log::Info(Event::Render,
                          "Setting bind group slot " + std::to_string(groupIndex) + " handle=" +
                              std::to_string(reinterpret_cast<uintptr_t>(bindGroup)));
            }
            wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, groupIndex, bindGroup, 0, nullptr);
        } else {
            Log::Warning(Event::Render,
                         "Missing bind group for slot " + std::to_string(groupIndex) + " in drawable " + getName());
        }
    }

    // Handle depth and stencil states (like Metal does)
    // For 3D mode, stenciling is handled by the layer group
    if (!is3D) {
        if (enableStencil) {
            // WebGPU will need to handle stencil state here
            // TODO: Implement stencil state handling similar to Metal
            // const auto stencilMode = parameters.stencilModeForClipping(tileID->toUnwrapped());
        }

        if (getEnableDepth()) {
            // WebGPU will need to handle depth state here
            // TODO: Implement depth state handling similar to Metal
            // const auto depthMode = parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType());
        }
    }

    // Draw indexed geometry - loop through segments (exactly like Metal does)
    int segmentCount = 0;
    for (const auto& seg_ : impl->segments) {
        const auto& segment = static_cast<DrawSegment&>(*seg_);
        const auto& mlSegment = segment.getSegment();
        if (mlSegment.indexLength > 0) {
            const uint32_t instanceCount = instanceAttributes ? instanceAttributes->getMaxCount() : 1;
            const uint32_t indexOffset = mlSegment.indexOffset;
            const int32_t baseVertex = static_cast<int32_t>(mlSegment.vertexOffset);
            const uint32_t baseInstance = 0;

            if (drawCallCount <= 200 && getName().find("fill") != std::string::npos) {
                Log::Info(Event::Render, "  FILL Drawing segment " + std::to_string(++segmentCount) +
                         " indices=" + std::to_string(mlSegment.indexLength) +
                         " indexOffset=" + std::to_string(indexOffset) +
                         " baseVertex=" + std::to_string(baseVertex) +
                         " instances=" + std::to_string(instanceCount));
            }

            // Check if encoder is valid before drawing
            if (!renderPassEncoder) {
                Log::Error(Event::Render, "Render pass encoder became null before draw!");
                return;
            }

            Log::Info(Event::Render, "Calling wgpuRenderPassEncoderDrawIndexed with encoder=" +
                     std::to_string(reinterpret_cast<uintptr_t>(renderPassEncoder)) +
                     " indexCount=" + std::to_string(mlSegment.indexLength));

            // Make sure we have valid indices
            if (mlSegment.indexLength == 0) {
                Log::Warning(Event::Render, "Skipping draw with 0 indices");
                continue;
            }

            wgpuRenderPassEncoderDrawIndexed(renderPassEncoder,
                                            mlSegment.indexLength,  // indexCount
                                            instanceCount,           // instanceCount
                                            indexOffset,             // firstIndex
                                            baseVertex,              // baseVertex
                                            baseInstance);           // firstInstance

            Log::Info(Event::Render, "wgpuRenderPassEncoderDrawIndexed call completed");

            context.renderingStats().numDrawCalls++;
        }
    }
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indices, std::vector<UniqueDrawSegment> segments) {
    impl->indexes = std::move(indices);
    impl->segments = std::move(segments);
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType dataType) {
    impl->vertexCount = count;
    impl->vertexType = dataType;

    if (count && dataType != gfx::AttributeDataType::Invalid && !data.empty()) {
        if (!vertexAttributes) {
            vertexAttributes = std::make_shared<gfx::VertexAttributeArray>();
        }
        if (auto& attrib = vertexAttributes->set(impl->vertexAttrId, /*index=*/-1, dataType)) {
            attrib->setRawData(std::move(data));
            attrib->setStride(gfx::VertexAttribute::getStrideOf(dataType));
        } else {
            using namespace std::string_literals;
            Log::Warning(Event::General,
                         "Vertex attribute type mismatch: "s + name + " / " + util::toString(impl->vertexAttrId));
            assert(false);
        }
    }
}

const gfx::UniformBufferArray& Drawable::getUniformBuffers() const {
    return impl->uniformBuffers;
}

gfx::UniformBufferArray& Drawable::mutableUniformBuffers() {
    return impl->uniformBuffers;
}

void Drawable::setVertexAttrId(const size_t id) {
    impl->vertexAttrId = id;
}

void Drawable::updateVertexAttributes(gfx::VertexAttributeArrayPtr vertices,
                                     std::size_t vertexCount,
                                     gfx::DrawMode mode,
                                     gfx::IndexVectorBasePtr indexes,
                                     const SegmentBase* segments,
                                     std::size_t segmentCount) {
    // Store the vertex attributes (base class method)
    gfx::Drawable::setVertexAttributes(std::move(vertices));

    // Store vertex count
    impl->vertexCount = vertexCount;

    // Handle segments - check for nullptr
    std::vector<UniqueDrawSegment> drawSegs;
    if (segments && segmentCount > 0) {
        drawSegs.reserve(segmentCount);
        for (std::size_t i = 0; i < segmentCount; ++i) {
            const auto& seg = segments[i];
            drawSegs.push_back(std::make_unique<DrawSegment>(mode, SegmentBase{
                seg.vertexOffset,
                seg.indexOffset,
                seg.vertexLength,
                seg.indexLength,
                seg.sortKey
            }));
        }
    }

    // Set the index data with segments
    setIndexData(std::move(indexes), std::move(drawSegs));

}

} // namespace webgpu
} // namespace mbgl
