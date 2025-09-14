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

    if (impl->vertexBuffer) {
        wgpuBufferRelease(impl->vertexBuffer);
        impl->vertexBuffer = nullptr;
    }

    if (impl->indexBuffer) {
        wgpuBufferRelease(impl->indexBuffer);
        impl->indexBuffer = nullptr;
    }

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
        // WebGPU requires interleaved vertex data in a single buffer
        // whereas Metal can use separate attribute buffers
        // For now, we'll handle this in a simplified way

        // TODO: Implement proper vertex attribute handling for WebGPU
        // This would involve extracting and interleaving the vertex data
        // from the individual attribute arrays
    }

    // Upload vertex data to GPU
    if (!impl->vertexData.empty()) {
        std::size_t bufferSize = impl->vertexData.size();

        // Align to 16 bytes as required by WebGPU
        const std::size_t alignment = 16;
        bufferSize = ((bufferSize + alignment - 1) / alignment) * alignment;

        // Release old buffer if it exists (safely)
        if (impl->vertexBuffer) {
            WGPUBuffer oldBuffer = impl->vertexBuffer;
            impl->vertexBuffer = nullptr;
            wgpuBufferRelease(oldBuffer);
        }

        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView vertexLabel = {"Vertex Buffer", strlen("Vertex Buffer")};
        bufferDesc.label = vertexLabel;
        bufferDesc.size = bufferSize;
        bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = 0; // Don't map at creation

        impl->vertexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);

        if (impl->vertexBuffer) {
            // Use queue write instead of mapping
            auto& webgpuContext = static_cast<webgpu::Context&>(context);
            WGPUDevice contextDevice = static_cast<WGPUDevice>(webgpuContext.getBackend().getDevice());
            WGPUQueue queue = wgpuDeviceGetQueue(contextDevice);
            if (queue) {
                // Write the actual data (padded with zeros if needed)
                if (impl->vertexData.size() < bufferSize) {
                    // Need to pad with zeros
                    std::vector<uint8_t> paddedData(bufferSize, 0);
                    std::memcpy(paddedData.data(), impl->vertexData.data(), impl->vertexData.size());
                    wgpuQueueWriteBuffer(queue, impl->vertexBuffer, 0, paddedData.data(), bufferSize);
                } else {
                    wgpuQueueWriteBuffer(queue, impl->vertexBuffer, 0, impl->vertexData.data(), impl->vertexData.size());
                }
            }
        }
    }

    // Upload index data to GPU
    if (impl->indexes && impl->indexes->elements() > 0) {
        std::size_t indexSize = impl->indexes->bytes();

        // Release old buffer if it exists (safely)
        if (impl->indexBuffer) {
            WGPUBuffer oldBuffer = impl->indexBuffer;
            impl->indexBuffer = nullptr;
            wgpuBufferRelease(oldBuffer);
        }

        // Create index buffer
        const void* indexData = impl->indexes->data();

        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView indexLabel = {"Index Buffer", strlen("Index Buffer")};
        bufferDesc.label = indexLabel;
        bufferDesc.size = indexSize;
        bufferDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = 0; // Don't map at creation

        impl->indexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);

        if (impl->indexBuffer) {
            // Use queue write instead of mapping
            auto& webgpuContext = static_cast<webgpu::Context&>(context);
            WGPUDevice contextDevice = static_cast<WGPUDevice>(webgpuContext.getBackend().getDevice());
            WGPUQueue queue = wgpuDeviceGetQueue(contextDevice);
            if (queue) {
                wgpuQueueWriteBuffer(queue, impl->indexBuffer, 0, indexData, indexSize);
            }
        }
    }
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
            // For now, just use the cached pipeline from the shader
            // TODO: In the future, we should create pipelines with proper vertex layouts
            // This would require either:
            // 1. Making webgpu::Drawable inherit from gfx::Renderable, or
            // 2. Modifying getRenderPipeline to not require a Renderable parameter
            impl->pipelineState = webgpuShader->getPipeline();

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

    // Set vertex buffer
    if (impl->vertexBuffer) {
        wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 0, impl->vertexBuffer, 0, WGPU_WHOLE_SIZE);
    }

    // Set index buffer
    if (impl->indexBuffer && impl->indexes) {
        WGPUIndexFormat indexFormat = WGPUIndexFormat_Uint16;
        // TODO: Add proper index type detection when available
        // For now assume 16-bit indices which is most common
        if (false) { // Placeholder for future index type detection
            indexFormat = WGPUIndexFormat_Uint32;
        }
        wgpuRenderPassEncoderSetIndexBuffer(renderPassEncoder, impl->indexBuffer, indexFormat, 0, WGPU_WHOLE_SIZE);
    }

    // Set bind group
    if (impl->bindGroup) {
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, impl->bindGroup, 0, nullptr);
    }

    // Draw
    if (impl->indexBuffer && impl->indexes) {
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
    } else if (impl->vertexBuffer && impl->vertexCount > 0) {
        // Draw non-indexed
        uint32_t vertexCount = static_cast<uint32_t>(impl->vertexCount);
        wgpuRenderPassEncoderDraw(renderPassEncoder, vertexCount, 1, 0, 0);
    }
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indices, std::vector<UniqueDrawSegment> segments) {
    impl->indexes = std::move(indices);
    impl->segments = std::move(segments);
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType) {
    impl->vertexData = std::move(data);
    impl->vertexCount = count;
    if (count > 0) {
        // Calculate stride from data size and count
        // This is a WebGPU-specific requirement
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