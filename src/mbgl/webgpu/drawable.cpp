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
#include <algorithm>
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

std::size_t hashStencilMode(const gfx::StencilMode& mode) {
    std::size_t hash = util::hash(static_cast<int>(mode.ref),
                                  static_cast<uint32_t>(mode.mask),
                                  static_cast<int>(mode.fail),
                                  static_cast<int>(mode.depthFail),
                                  static_cast<int>(mode.pass));
    mapbox::util::apply_visitor(
        [&](const auto& test) {
            using TestType = std::decay_t<decltype(test)>;
            hash = util::hash(hash, static_cast<int>(TestType::func));
            if constexpr (requires { test.mask; }) {
                hash = util::hash(hash, static_cast<uint32_t>(test.mask));
            }
        },
        mode.test);
    return hash;
}

std::size_t hashPipelineState(const std::vector<WGPUVertexBufferLayout>& vertexLayouts,
                              const gfx::ColorMode& colorMode,
                              const gfx::DepthMode& depthMode,
                              const gfx::StencilMode& stencilMode,
                              gfx::DrawModeType drawModeType) {
    std::size_t hash = 0;

    // Hash vertex layouts
    for (const auto& layout : vertexLayouts) {
        hash = util::hash(hash, static_cast<uint32_t>(layout.arrayStride));
        hash = util::hash(hash, static_cast<uint32_t>(layout.stepMode));
        hash = util::hash(hash, static_cast<uint32_t>(layout.attributeCount));
        for (uint32_t i = 0; i < layout.attributeCount && i < 16; ++i) {
            hash = util::hash(hash,
                              static_cast<uint32_t>(layout.attributes[i].format),
                              static_cast<uint32_t>(layout.attributes[i].shaderLocation),
                              static_cast<uint32_t>(layout.attributes[i].offset));
        }
    }

    // Hash color mode
    hash = util::hash(hash, colorMode.mask.r, colorMode.mask.g, colorMode.mask.b, colorMode.mask.a);
    colorMode.blendFunction.match([&](const auto& blendFunc) {
        using T = std::decay_t<decltype(blendFunc)>;
        if constexpr (std::is_same_v<T, gfx::ColorMode::Replace>) {
            hash = util::hash(hash, 0);
        } else {
            hash = util::hash(hash,
                              static_cast<int>(blendFunc.equation),
                              static_cast<int>(blendFunc.srcFactor),
                              static_cast<int>(blendFunc.dstFactor));
        }
    });

    // Hash depth mode
    hash = util::hash(hash, static_cast<int>(depthMode.func), static_cast<int>(depthMode.mask));

    // Hash stencil mode
    hash = util::hash(hash, hashStencilMode(stencilMode));

    // Hash draw mode
    hash = util::hash(hash, static_cast<int>(drawModeType));

    return hash;
}

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

    for (auto& record : impl->bindGroups) {
        if (record.handle) {
            wgpuBindGroupRelease(record.handle);
        }
    }
    impl->bindGroups.clear();

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

void Drawable::setColorMode(const gfx::ColorMode& value) {
    gfx::Drawable::setColorMode(value);
    impl->pipelineState = nullptr; // Rebuild pipeline to honour new color mode
}

void Drawable::setEnableStencil(bool value) {
    if (getEnableStencil() == value) {
        return;
    }
    gfx::Drawable::setEnableStencil(value);
    impl->pipelineState = nullptr; // Reset pipeline to force recreation with new stencil state
}

void Drawable::setEnableDepth(bool value) {
    if (getEnableDepth() == value) {
        return;
    }
    gfx::Drawable::setEnableDepth(value);
    impl->pipelineState = nullptr; // Reset pipeline to force recreation with new depth state
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
    impl->pipelineState = nullptr; // Reset pipeline to force recreation with new depth mask
}

void Drawable::setDepthModeFor3D(const gfx::DepthMode& value) {
    if (impl->depthMode.func != value.func || impl->depthMode.mask != value.mask) {
        impl->pipelineState = nullptr;
    }
    impl->depthMode = value;
}

void Drawable::setStencilModeFor3D(const gfx::StencilMode& value) {
    const auto newHash = hashStencilMode(value);
    if (!impl->stencilModeHashValid || impl->stencilModeHash != newHash || impl->stencilMode.ref != value.ref ||
        impl->stencilMode.mask != value.mask || impl->stencilMode.fail != value.fail ||
        impl->stencilMode.depthFail != value.depthFail || impl->stencilMode.pass != value.pass) {
        impl->pipelineState = nullptr;
        impl->stencilModeHashValid = false;
    }
    impl->stencilMode = value;
    impl->stencilModeHash = newHash;
}

void Drawable::setShader(gfx::ShaderProgramBasePtr value) {
    if (shader == value) {
        return;
    }
    shader = std::move(value);
    impl->pipelineState = nullptr; // Reset pipeline when shader changes
}

void Drawable::upload(gfx::UploadPass& uploadPass) {
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
        auto indexBufferResource = webgpuUploadPass.createIndexBufferResource(impl->indexes->data(),
                                                                              impl->indexes->bytes(),
                                                                              usage,
                                                                              /*persistent=*/false);
        auto gfxIndexBuffer = std::make_unique<gfx::IndexBuffer>(impl->indexes->elements(),
                                                                 std::move(indexBufferResource));
        auto buffer = std::make_unique<IndexBuffer>(std::move(gfxIndexBuffer));

        impl->indexes->setBuffer(std::move(buffer));
        impl->indexes->setDirty(false);
    }

    // Upload vertex attributes (like Metal does)
    const bool buildAttribs = !vertexAttributes || !attributeUpdateTime ||
                              vertexAttributes->isModifiedAfter(*attributeUpdateTime);

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
            impl->vertexDescHash = 0;      // Reset hash when bindings change
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

            for (auto& record : impl->bindGroups) {
                if (record.handle) {
                    wgpuBindGroupRelease(record.handle);
                }
            }
            impl->bindGroups.clear();

            if (deviceHandle) {
                const auto& groupOrder = webgpuShader->getBindGroupOrder();
                static bool loggedBindGroupOrder = false;
                if (!loggedBindGroupOrder && getName().find("fill-outline") != std::string::npos) {
                    std::string orderStr;
                    for (size_t idx = 0; idx < groupOrder.size(); ++idx) {
                        orderStr += std::to_string(groupOrder[idx]);
                        if (idx + 1 < groupOrder.size()) orderStr += ",";
                    }
                    loggedBindGroupOrder = true;
                }
                const auto* globalBuffers = webgpuRenderPass.getGlobalUniformBuffers();

                const auto findTextureForBinding = [&](uint32_t binding, bool samplerBinding) -> Texture2D* {
                    for (size_t texIndex = 0; texIndex < textures.size(); ++texIndex) {
                        if (!textures[texIndex]) {
                            continue;
                        }
                        if (const auto location = webgpuShader->getSamplerLocation(texIndex)) {
                            if ((samplerBinding && *location == binding) ||
                                (!samplerBinding && (*location == binding || *location + 1 == binding))) {
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
                        const std::string shaderName = webgpuShader ? std::string(webgpuShader->typeName()) : "unknown";
                        Log::Warning(Event::Render,
                                     "WebGPU: missing bind group layout for group " + std::to_string(group) +
                                         " in shader '" + shaderName + "'");
                        continue;
                    }

                    const auto& bindingInfos = webgpuShader->getBindingInfosForGroup(group);
                    if (getName().find("fill-outline") != std::string::npos) {
                        std::string infoStr;
                        for (const auto& info : bindingInfos) {
                            infoStr += "(binding=" + std::to_string(info.binding) +
                                       ", type=" + std::to_string(static_cast<int>(info.type)) + ")";
                        }
                    }
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
                                std::shared_ptr<gfx::UniformBuffer> buffer = impl->uniformBuffers.get(
                                    bindingInfo.binding);
                                if (bindingInfo.binding == shaders::idGlobalUBOIndex) {
                                    GlobalUBOIndexData indexData;
                                    indexData.value = getUBOIndex();
                                    context.emplaceOrUpdateUniformBuffer(
                                        impl->uboIndexUniform, &indexData, sizeof(indexData), false);
                                    if (impl->uboIndexUniform) {
                                        impl->uniformBuffers.set(bindingInfo.binding, impl->uboIndexUniform);
                                        buffer = impl->uboIndexUniform;
                                    }
                                }

                                if (!buffer && globalBuffers) {
                                    buffer = globalBuffers->get(bindingInfo.binding);
                                }

                                if (!buffer) {
                                    if (drawCallCount <= 200) {
                                        Log::Warning(Event::Render,
                                                     "No buffer found for binding " +
                                                         std::to_string(bindingInfo.binding) +
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
                                                         std::to_string(bindingInfo.binding) +
                                                         " group=" + std::to_string(group));
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
                        Log::Warning(Event::Render,
                                     "WebGPU: invalid bindings for drawable '" + getName() +
                                         "' (group=" + std::to_string(group) + ", slot=" + std::to_string(slot) + ")");
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
                    impl->bindGroups.push_back({static_cast<uint32_t>(slot), group, bindGroup});
                }
            }
        }
    }

    const auto drawableTileID = getTileID();

    gfx::DepthMode depthMode = gfx::DepthMode::disabled();
    gfx::StencilMode stencilMode = gfx::StencilMode::disabled();

    if (is3D) {
        depthMode = impl->depthMode;
        stencilMode = impl->stencilMode;
    } else {
        if (getEnableDepth()) {
            depthMode = parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType());
        }
        if (getEnableStencil() && drawableTileID) {
            stencilMode = parameters.stencilModeForClipping(drawableTileID->toUnwrapped());
        }
    }

    const std::size_t newStencilHash = hashStencilMode(stencilMode);
    if (!impl->stencilModeHashValid || impl->stencilModeHash != newStencilHash ||
        impl->depthMode.func != depthMode.func || impl->depthMode.mask != depthMode.mask) {
        impl->pipelineState = nullptr;
        impl->stencilModeHashValid = true;
    }

    impl->depthMode = depthMode;
    impl->stencilMode = stencilMode;
    impl->stencilModeHash = newStencilHash;
    impl->previousStencilMode = stencilMode;

    // Get pipeline from shader if we don't have it yet (like Metal)
    struct UniqueBinding {
        const VertexBufferResource* vertexResource = nullptr;
        const BufferResource* buffer = nullptr;
        uint32_t stride = 0;
        uint64_t baseOffset = 0;
        WGPUVertexStepMode stepMode = WGPUVertexStepMode_Vertex;
        std::vector<WGPUVertexAttribute> attributes;
        std::vector<uint64_t> attributeByteOffsets;
    };

    auto accumulateBindings = [](const gfx::AttributeBindingArray& source,
                                 const WGPUVertexStepMode stepMode,
                                 std::vector<UniqueBinding>& out) {
        for (size_t attributeIndex = 0; attributeIndex < source.size(); ++attributeIndex) {
            const auto& binding = source[attributeIndex];
            if (!binding.has_value() || !binding->vertexBufferResource) {
                continue;
            }

            const auto* vertexBufferRes = static_cast<const VertexBufferResource*>(binding->vertexBufferResource);
            if (!vertexBufferRes) {
                continue;
            }

            const auto& bufferResource = vertexBufferRes->getBuffer();
            if (!bufferResource.getBuffer()) {
                continue;
            }

            const uint64_t bindingByteOffset = static_cast<uint64_t>(binding->vertexOffset) * binding->vertexStride;

            auto it = std::find_if(out.begin(), out.end(), [&](const UniqueBinding& entry) {
                return entry.vertexResource == vertexBufferRes && entry.stride == binding->vertexStride &&
                       entry.stepMode == stepMode;
            });

            if (it == out.end()) {
                UniqueBinding entry;
                entry.vertexResource = vertexBufferRes;
                entry.buffer = &bufferResource;
                entry.stride = binding->vertexStride;
                entry.stepMode = stepMode;
                entry.baseOffset = bindingByteOffset;
                entry.attributes.reserve(4);
                entry.attributeByteOffsets.reserve(4);
                out.push_back(std::move(entry));
                it = out.end() - 1;
            } else {
                it->baseOffset = std::min(it->baseOffset, bindingByteOffset);
            }

            WGPUVertexAttribute attr = {};
            attr.format = wgpuVertexFormatOf(binding->attribute.dataType);
            attr.shaderLocation = static_cast<uint32_t>(attributeIndex);
            attr.offset = binding->attribute.offset;

            it->attributes.push_back(attr);
            it->attributeByteOffsets.push_back(bindingByteOffset);
        }
    };

    std::vector<UniqueBinding> uniqueBindings;
    uniqueBindings.reserve(impl->attributeBindings.size() + impl->instanceBindings.size());
    accumulateBindings(impl->attributeBindings, WGPUVertexStepMode_Vertex, uniqueBindings);
    accumulateBindings(impl->instanceBindings, WGPUVertexStepMode_Instance, uniqueBindings);

    constexpr uint32_t maxVertexBufferSlots = 8;
    if (uniqueBindings.size() > maxVertexBufferSlots) {
        Log::Warning(Event::Render,
                     "WebGPU: reducing vertex buffer bindings from " + std::to_string(uniqueBindings.size()) + " to " +
                         std::to_string(maxVertexBufferSlots));
        uniqueBindings.resize(maxVertexBufferSlots);
    }

    for (auto& entry : uniqueBindings) {
        for (size_t j = 0; j < entry.attributes.size(); ++j) {
            if (entry.attributeByteOffsets[j] < entry.baseOffset) {
                continue;
            }
            const auto delta = entry.attributeByteOffsets[j] - entry.baseOffset;
            entry.attributes[j].offset += static_cast<uint32_t>(delta);
        }
    }

    auto& shaderWebGPU = static_cast<mbgl::webgpu::ShaderProgram&>(*shader);

    if (!impl->pipelineState) {
        // Create vertex layout from attribute bindings
        std::vector<WGPUVertexBufferLayout> vertexLayouts;
        vertexLayouts.reserve(uniqueBindings.size());

        for (auto& binding : uniqueBindings) {
            WGPUVertexBufferLayout layout = {};
            layout.arrayStride = binding.stride;
            layout.stepMode = binding.stepMode;
            layout.attributeCount = binding.attributes.size();
            layout.attributes = binding.attributes.data();
            vertexLayouts.push_back(layout);
        }

        // Get render pipeline similar to Metal's getRenderPipelineState
        // Use the actual color mode from the drawable (like Metal does)
        const auto& colorMode = getColorMode();

        // Get the renderable from render pass descriptor like Metal does
        const auto& renderable = webgpuRenderPass.getDescriptor().renderable;

        gfx::DrawModeType drawModeType = gfx::DrawModeType::Triangles;
        if (!impl->segments.empty()) {
            drawModeType = impl->segments.front()->getMode().type;
        }

        // Compute pipeline state hash for caching
        const std::size_t pipelineHash = hashPipelineState(
            vertexLayouts, colorMode, depthMode, stencilMode, drawModeType);

        impl->pipelineState = shaderWebGPU.getRenderPipeline(renderable,
                                                             vertexLayouts.empty() ? nullptr : vertexLayouts.data(),
                                                             vertexLayouts.size(),
                                                             colorMode,
                                                             depthMode,
                                                             stencilMode,
                                                             drawModeType,
                                                             pipelineHash);
        if (!impl->pipelineState) {
            Log::Warning(Event::Render,
                         "WebGPU: skipping pipeline creation for drawable '" + getName() + "' (shader='" +
                             shaderWebGPU.typeName().data() + "')");
        }
    }

    if (impl->pipelineState) {
        if (!renderPassEncoder) {
            Log::Error(Event::Render, "renderPassEncoder is null!");
            return;
        }
        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, impl->pipelineState);
        if (getEnableStencil()) {
            wgpuRenderPassEncoderSetStencilReference(renderPassEncoder, static_cast<uint32_t>(stencilMode.ref));
        }
    } else {
        Log::Warning(Event::Render, "WebGPU: no pipeline available for drawable '" + getName() + "'; draw skipped");
        return;
    }

    wgpuRenderPassEncoderSetScissorRect(renderPassEncoder,
                                        parameters.scissorRect.x,
                                        parameters.scissorRect.y,
                                        parameters.scissorRect.width,
                                        parameters.scissorRect.height);

    uint32_t bufferSlot = 0;
    for (const auto& binding : uniqueBindings) {
        if (!binding.buffer || !binding.buffer->getBuffer()) {
            continue;
        }

        const uint64_t bufferSize = binding.buffer->getSizeInBytes();
        if (binding.baseOffset >= bufferSize) {
            Log::Warning(Event::Render,
                         "Skipping vertex buffer slot " + std::to_string(bufferSlot) + " due to base offset " +
                             std::to_string(binding.baseOffset) + " >= size " + std::to_string(bufferSize));
            continue;
        }

        wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder,
                                             bufferSlot,
                                             binding.buffer->getBuffer(),
                                             binding.baseOffset,
                                             bufferSize - binding.baseOffset);
        bufferSlot++;
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
    for (const auto& record : impl->bindGroups) {
        if (record.handle) {
            wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, record.slot, record.handle, 0, nullptr);
        }
    }

    // Handle depth and stencil states (like Metal does)
    // For 3D mode, stenciling is handled by the layer group
    if (!is3D) {
        if (enableStencil) {
            if (tileID) {
                const auto expectedStencil = parameters.stencilModeForClipping(tileID->toUnwrapped());
#if !defined(NDEBUG)
                assert(hashStencilMode(expectedStencil) == hashStencilMode(impl->stencilMode));
#else
                if (hashStencilMode(expectedStencil) != hashStencilMode(impl->stencilMode)) {
                    Log::Warning(
                        Event::Render,
                        "WebGPU drawable stencil mismatch for '" + getName() + "' tile=" + util::toString(*tileID));
                }
#endif
                wgpuRenderPassEncoderSetStencilReference(renderPassEncoder, static_cast<uint32_t>(expectedStencil.ref));
            }
        } else {
            // Reset the reference so subsequent drawables that re-enable stencilling start from a known value.
            wgpuRenderPassEncoderSetStencilReference(renderPassEncoder, 0u);
        }

        if (getEnableDepth()) {
#if !defined(NDEBUG)
            const auto expectedDepth = parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType());
            assert(expectedDepth.func == impl->depthMode.func);
            assert(expectedDepth.mask == impl->depthMode.mask);
#endif
        }
    }

    // Draw indexed geometry - loop through segments (exactly like Metal does)
    // int segmentCount = 0;
    for (const auto& seg_ : impl->segments) {
        const auto& segment = static_cast<DrawSegment&>(*seg_);
        const auto& mlSegment = segment.getSegment();
        if (mlSegment.indexLength > 0) {
            const uint32_t instanceCount = instanceAttributes ? instanceAttributes->getMaxCount() : 1;
            const uint32_t indexOffset = mlSegment.indexOffset;
            const int32_t baseVertex = static_cast<int32_t>(mlSegment.vertexOffset);
            const uint32_t baseInstance = 0;

            // Check if encoder is valid before drawing
            if (!renderPassEncoder) {
                Log::Error(Event::Render, "Render pass encoder became null before draw!");
                return;
            }

            // Make sure we have valid indices
            if (mlSegment.indexLength == 0) {
                Log::Warning(Event::Render, "Skipping draw with 0 indices");
                continue;
            }

            wgpuRenderPassEncoderDrawIndexed(renderPassEncoder,
                                             mlSegment.indexLength, // indexCount
                                             instanceCount,         // instanceCount
                                             indexOffset,           // firstIndex
                                             baseVertex,            // baseVertex
                                             baseInstance);         // firstInstance

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
            drawSegs.push_back(std::make_unique<DrawSegment>(
                mode, SegmentBase{seg.vertexOffset, seg.indexOffset, seg.vertexLength, seg.indexLength, seg.sortKey}));
        }
    }

    // Set the index data with segments
    setIndexData(std::move(indexes), std::move(drawSegs));
}

} // namespace webgpu
} // namespace mbgl
