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

#include <cassert>
#include <memory>
#if !defined(NDEBUG)
#include <sstream>
#endif

namespace mbgl {
namespace webgpu {

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
    auto& webgpuUploadPass = static_cast<webgpu::UploadPass&>(uploadPass);
    auto& gfxContext = webgpuUploadPass.getContext();
    auto& context = static_cast<webgpu::Context&>(gfxContext);
    auto& backend = static_cast<webgpu::RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (!device) {
        return;
    }

    // Handle index buffer creation like Metal does
    if (impl->indexes && (!impl->indexes->getBuffer() || impl->indexes->getDirty())) {
        if (!impl->indexes->empty()) {
            // Create index buffer resource
            auto indexBufferResource = webgpuUploadPass.createIndexBufferResource(
                impl->indexes->data(),
                impl->indexes->bytes(),
                gfx::BufferUsageType::StaticDraw,
                /*persistent=*/false);

            // Create gfx::IndexBuffer
            auto gfxIndexBuffer = std::make_unique<gfx::IndexBuffer>(
                impl->indexes->elements(),
                std::move(indexBufferResource));

            // Wrap it in local IndexBuffer wrapper
            auto indexBuffer = std::make_unique<IndexBuffer>(std::move(gfxIndexBuffer));

            impl->indexes->setBuffer(std::move(indexBuffer));
            impl->indexes->setDirty(false);
        }
    }

    // Store the bind group layout if available for later bind group creation
    if (impl->bindGroupLayout && !impl->bindGroup && shader) {
        auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
        if (webgpuShader) {
            impl->bindGroupLayout = webgpuShader->getBindGroupLayout();
        }
    }
    // Bind group will be created during draw() using the UniformBufferArray

    // Process vertex attributes like Metal does
    if (vertexAttributes) {
        // Build attribute bindings from the vertex attributes
        if (!vertexAttributes) {
            vertexAttributes = std::make_shared<gfx::VertexAttributeArray>();
        }

        // Build vertex buffers from the vertex attributes
        std::vector<std::unique_ptr<gfx::VertexBufferResource>> vertexBuffers;
        auto attributeBindings_ = webgpuUploadPass.buildAttributeBindings(
            impl->vertexCount,
            gfx::AttributeDataType::Invalid,  // vertexType
            /*vertexAttributeIndex=*/-1,
            /*vertexData=*/{},
            shader->getVertexAttributes(),
            *vertexAttributes,
            gfx::BufferUsageType::DynamicDraw,
            std::nullopt,  // attributeUpdateTime
            vertexBuffers);

        // Mark attributes as clean
        vertexAttributes->visitAttributes([](gfx::VertexAttribute& attrib) { attrib.setDirty(false); });

        // Update attribute bindings if changed
        if (impl->attributeBindings != attributeBindings_) {
            impl->attributeBindings = std::move(attributeBindings_);
            // For now, we'll use the pre-existing vertex data setup
            // Proper interleaving can be implemented later when we have access
            // to the vertex buffer data
        }
    }

    // Note: Vertex data is now handled through attributeBindings, similar to Metal.
    // The vertex buffers are created in buildAttributeBindings and stored in impl->attributeBindings.

    // Note: Index data upload is handled separately when needed.
    // The index buffer is part of the IndexVectorBase and will be uploaded
    // through the upload pass mechanism.
}

void Drawable::draw(PaintParameters& parameters) const {
    if (isCustom) {
        return;
    }

    // Get the render pass
    if (!parameters.renderPass) {
        return;
    }

    // Get the actual WebGPU render pass encoder
    auto* webgpuRenderPass = static_cast<webgpu::RenderPass*>(parameters.renderPass.get());
    if (!webgpuRenderPass) {
        return;
    }

    WGPURenderPassEncoder renderPassEncoder = webgpuRenderPass->getEncoder();
    if (!renderPassEncoder) {
        return;
    }

    if (!shader) {
        return;
    }

    if (!getEnabled()) {
        return;
    }

    // Get WebGPU context and device
    auto& context = static_cast<webgpu::Context&>(parameters.context);
    auto& backend = static_cast<webgpu::RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());

    if (!device || !queue) {
        return;
    }

    // Cast to WebGPU shader
    // const auto& shaderWebGPU = static_cast<const mbgl::webgpu::ShaderProgram&>(*shader);

    // Call bind on UniformBufferArray to ensure buffers are ready (like Metal does)
    impl->uniformBuffers.bind(*parameters.renderPass);

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
                        }
                    }
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

    // Get pipeline from shader if we don't have it yet
    if (!impl->pipelineState) {
        // Use dynamic_pointer_cast for safer casting with RTTI
        auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
        if (webgpuShader) {
            // Create vertex layout from attribute bindings
            std::vector<WGPUVertexBufferLayout> vertexLayouts;
            std::vector<std::vector<WGPUVertexAttribute>> vertexAttrs;

            // Create vertex buffer layouts from attribute bindings
            uint32_t bufferIndex = 0;
            for (const auto& binding : impl->attributeBindings) {
                if (!binding.has_value() || !binding->vertexBufferResource) {
                    bufferIndex++;
                    continue;
                }

                // Create vertex attribute for this buffer
                vertexAttrs.push_back({});
                auto& attrs = vertexAttrs.back();

                WGPUVertexAttribute attr = {};
                // TODO: Determine proper format from binding data type
                attr.format = WGPUVertexFormat_Float32x2; // Default format
                attr.offset = binding->vertexOffset;
                attr.shaderLocation = bufferIndex;
                attrs.push_back(attr);

                // Create vertex buffer layout
                WGPUVertexBufferLayout layout = {};
                layout.arrayStride = binding->vertexStride;
                layout.stepMode = WGPUVertexStepMode_Vertex;
                layout.attributeCount = attrs.size();
                layout.attributes = attrs.data();
                vertexLayouts.push_back(layout);

                bufferIndex++;
            }

            // Try to get or create pipeline with vertex layouts
            impl->pipelineState = webgpuShader->getRenderPipeline(
                vertexLayouts.empty() ? nullptr : vertexLayouts.data(),
                vertexLayouts.size());

            if (!impl->pipelineState) {
                // Fallback to cached pipeline if custom creation fails
                impl->pipelineState = webgpuShader->getPipeline();
            }

            if (!impl->pipelineState) {
                return;
            }
        } else {
            return;
        }
    }

    // Set the pipeline
    if (impl->pipelineState) {
        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, impl->pipelineState);
    } else {
        return;
    }

    // Bind vertex buffers from attributeBindings (like Metal does)
    uint32_t attributeIndex = 0;
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
                        binding->vertexOffset,
                        buffer.getSizeInBytes());
                }
            }
        }
        attributeIndex++;
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
    if (impl->bindGroup) {
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, impl->bindGroup, 0, nullptr);
    }

    // Draw
    if (impl->indexes && impl->indexes->elements() > 0 && !impl->indexes->getDirty()) {
        // Draw indexed geometry - loop through segments like Metal does
        for (const auto& seg_ : impl->segments) {
            const auto& segment = static_cast<DrawSegment&>(*seg_);
            const auto& mlSegment = segment.getSegment();
            if (mlSegment.indexLength > 0) {
                uint32_t indexCount = mlSegment.indexLength;
                uint32_t indexOffset = mlSegment.indexOffset;
                uint32_t baseVertex = mlSegment.vertexOffset;

                wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, indexCount, 1, indexOffset, baseVertex, 0);
            }
        }
    } else if (!impl->attributeBindings.empty() && impl->vertexCount > 0) {
        // Draw non-indexed
        uint32_t vertexCount = static_cast<uint32_t>(impl->vertexCount);
        wgpuRenderPassEncoderDraw(renderPassEncoder, vertexCount, 1, 0, 0);
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