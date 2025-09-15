#include <mbgl/webgpu/drawable.hpp>

#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/segment.hpp>
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

    if (impl->bindGroup) {
        wgpuBindGroupRelease(impl->bindGroup);
        impl->bindGroup = nullptr;
    }

    // Uniform buffers are managed by UniformBufferArray
    // Vertex and index buffers are now managed through attributeBindings and indexes

    // Clear the bind group layout reference (we don't own it)
    impl->bindGroupLayout = nullptr;
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
std::string debugLabel(const gfx::Drawable& drawable) {
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

    std::stringstream uploadMsg;
    uploadMsg << "WebGPU upload() buildAttribs=" << (buildAttribs ? "true" : "false")
              << ", attributeUpdateTime=" << (attributeUpdateTime ? "set" : "null");
    Log::Info(Event::Render, uploadMsg.str());

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
        }
    }

    // Upload uniform buffers to ensure they're ready for the GPU
    // This is critical for making drawables visible
    Log::Info(Event::Render, "Checking uniform buffers, allocated size: " +
              std::to_string(impl->uniformBuffers.allocatedSize()));

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
                } else {
                    Log::Info(Event::Render, "Uniform buffer " + std::to_string(i) + " is ready, size: " +
                             std::to_string(webgpuUniformBuffer->getSize()));
                }
            }
        } else {
            Log::Info(Event::Render, "Uniform buffer slot " + std::to_string(i) + " is empty");
        }
    }

    // Clear any existing bind group to force recreation with updated buffers
    if (impl->bindGroup) {
        wgpuBindGroupRelease(impl->bindGroup);
        impl->bindGroup = nullptr;
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
    // Log::Info(Event::Render, "WebGPU Drawable::draw called for " + getName());

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

    // Create bind group from UniformBufferArray if needed
    if (!impl->bindGroup && shader) {
        auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
        if (webgpuShader) {
            WGPUBindGroupLayout layout = webgpuShader->getBindGroupLayout();
            if (layout) {
                // Create bind group entries from UniformBufferArray
                std::vector<WGPUBindGroupEntry> entries;

                // WebGPU shaders now use the same binding indices as Metal/Vulkan
                // No remapping needed - just use the buffer index directly as the binding index
                for (size_t i = 0; i < impl->uniformBuffers.allocatedSize(); ++i) {
                    const auto& uniformBuffer = impl->uniformBuffers.get(i);
                    if (uniformBuffer) {
                        const auto* webgpuUniformBuffer = static_cast<const webgpu::UniformBuffer*>(uniformBuffer.get());
                        if (webgpuUniformBuffer && webgpuUniformBuffer->getBuffer()) {
                            WGPUBindGroupEntry entry = {};
                            entry.binding = static_cast<uint32_t>(i);
                            entry.buffer = webgpuUniformBuffer->getBuffer();
                            entry.offset = 0;
                            entry.size = webgpuUniformBuffer->getSize();
                            entries.push_back(entry);

                            Log::Info(Event::Render, "Adding uniform buffer to bind group: binding=" +
                                     std::to_string(i) + " size=" + std::to_string(entry.size));
                        }
                    }
                }

                if (entries.empty()) {
                    Log::Warning(Event::Render, "No uniform buffers found for bind group!");
                }

                // Add texture bindings if any
                for (size_t i = 0; i < textures.size(); ++i) {
                    if (textures[i]) {
                        // Texture binding logic would go here
                        // This requires proper texture view creation
                    }
                }

                // Create bind group descriptor
                WGPUBindGroupDescriptor bgDesc = {};
                WGPUStringView bgLabel = {"Drawable Bind Group", strlen("Drawable Bind Group")};
                bgDesc.label = bgLabel;
                bgDesc.layout = layout;
                bgDesc.entryCount = entries.size();
                bgDesc.entries = entries.data();

                // Create the bind group
                impl->bindGroup = wgpuDeviceCreateBindGroup(device, &bgDesc);
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
        uint32_t bufferIndex = 0;
        for (const auto& binding : impl->attributeBindings) {
            if (!binding.has_value() || !binding->vertexBufferResource) {
                bufferIndex++;
                continue;
            }

            // Create vertex attribute for this buffer
            vertexAttrs.emplace_back();
            auto& attrs = vertexAttrs.back();

            WGPUVertexAttribute attr = {};
            attr.format = wgpuVertexFormatOf(binding->attribute.dataType);
            attr.offset = binding->attribute.offset;
            attr.shaderLocation = bufferIndex;
            attrs.push_back(attr);

            bufferIndex++;
        }

        // Now create the layouts with stable pointers to attributes
        bufferIndex = 0;
        size_t attrIndex = 0;
        for (const auto& binding : impl->attributeBindings) {
            if (!binding.has_value() || !binding->vertexBufferResource) {
                bufferIndex++;
                continue;
            }

            WGPUVertexBufferLayout layout = {};
            layout.arrayStride = binding->vertexStride;
            layout.stepMode = WGPUVertexStepMode_Vertex;
            layout.attributeCount = vertexAttrs[attrIndex].size();
            layout.attributes = vertexAttrs[attrIndex].data();
            vertexLayouts.push_back(layout);

            attrIndex++;
            bufferIndex++;
        }

        // Get render pipeline similar to Metal's getRenderPipelineState
        // WebGPU doesn't have the same descriptor pattern as Metal
        // We'll use the simpler overload that doesn't need a renderable
        // Log::Info(Event::Render, "Creating render pipeline with " + std::to_string(vertexLayouts.size()) + " vertex layouts");
        impl->pipelineState = shaderWebGPU.getRenderPipeline(
            vertexLayouts.empty() ? nullptr : vertexLayouts.data(),
            vertexLayouts.size());
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
        // Log::Info(Event::Render, "Setting pipeline state: encoder=" +
        //     std::to_string(reinterpret_cast<uintptr_t>(renderPassEncoder)) +
        //     ", pipeline=" + std::to_string(reinterpret_cast<uintptr_t>(impl->pipelineState)));
        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, impl->pipelineState);
    } else {
        Log::Error(Event::Render, "Failed to create render pipeline state");
        assert(!"Failed to create render pipeline state");
        return;
    }


    // Bind vertex buffers from attributeBindings (like Metal does)
    uint32_t attributeIndex = 0;
    int boundBuffers = 0;
    Log::Info(Event::Render, "Binding vertex buffers, total bindings: " + std::to_string(impl->attributeBindings.size()));
    for (const auto& binding : impl->attributeBindings) {
        if (binding.has_value() && binding->vertexBufferResource) {
            const auto* vertexBufferRes = static_cast<const VertexBufferResource*>(binding->vertexBufferResource);
            if (vertexBufferRes) {
                const auto& buffer = vertexBufferRes->getBuffer();
                if (buffer.getBuffer()) {
                    wgpuRenderPassEncoderSetVertexBuffer(
                        renderPassEncoder,
                        attributeIndex,
                        buffer.getBuffer(),
                        binding->attribute.offset,
                        buffer.getSizeInBytes());
                    boundBuffers++;
                }
            }
        }
        attributeIndex++;
    }
    Log::Info(Event::Render, "Bound " + std::to_string(boundBuffers) + " vertex buffers");

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
    if (impl->bindGroup) {
        Log::Info(Event::Render, "Setting bind group");
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, impl->bindGroup, 0, nullptr);
    } else {
        Log::Warning(Event::Render, "No bind group available!");
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
    Log::Info(Event::Render, "Drawing " + std::to_string(impl->segments.size()) + " segments");
    for (const auto& seg_ : impl->segments) {
        const auto& segment = static_cast<DrawSegment&>(*seg_);
        const auto& mlSegment = segment.getSegment();
        if (mlSegment.indexLength > 0) {
            const uint32_t instanceCount = instanceAttributes ? instanceAttributes->getMaxCount() : 1;
            const uint32_t indexOffset = mlSegment.indexOffset;
            const int32_t baseVertex = static_cast<int32_t>(mlSegment.vertexOffset);
            const uint32_t baseInstance = 0;

            Log::Info(Event::Render, "DrawIndexed: indices=" + std::to_string(mlSegment.indexLength) +
                                    " instances=" + std::to_string(instanceCount) +
                                    " offset=" + std::to_string(indexOffset) +
                                    " baseVertex=" + std::to_string(baseVertex));

            wgpuRenderPassEncoderDrawIndexed(renderPassEncoder,
                                            mlSegment.indexLength,  // indexCount
                                            instanceCount,           // instanceCount
                                            indexOffset,             // firstIndex
                                            baseVertex,              // baseVertex
                                            baseInstance);           // firstInstance

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