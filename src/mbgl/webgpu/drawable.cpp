#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

class Drawable::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    // WebGPU resources
    WGPUBuffer vertexBuffer = nullptr;
    WGPUBuffer indexBuffer = nullptr;
    WGPURenderPipeline pipeline = nullptr;
    WGPUBindGroup bindGroup = nullptr;
    
    // Buffer data
    std::vector<uint8_t> vertexData;
    std::size_t vertexCount = 0;
    std::size_t vertexSize = 0;
    
    // Pipeline state
    bool colorEnabled = true;
    bool depthEnabled = true;
    gfx::DepthMaskType depthMask = gfx::DepthMaskType::ReadWrite;
    gfx::ColorMode colorMode;
    gfx::CullFaceMode cullFaceMode;
    
    // Uniform buffers
    std::unique_ptr<gfx::UniformBufferArray> uniformBuffers;
    
    // Draw segments are handled differently in WebGPU
    // We use the entire index buffer instead of segments
    gfx::IndexVectorBasePtr indexVector;
};

Drawable::Drawable(std::string name)
    : gfx::Drawable(std::move(name)),
      impl(std::make_unique<Impl>()) {
    // Initialize uniform buffers
    impl->uniformBuffers = std::make_unique<UniformBufferArray>();
}

Drawable::~Drawable() {
    // Clean up WebGPU resources
    if (impl->vertexBuffer) {
        // wgpuBufferDestroy(impl->vertexBuffer);
    }
    if (impl->indexBuffer) {
        // wgpuBufferDestroy(impl->indexBuffer);
    }
    if (impl->pipeline) {
        // wgpuRenderPipelineRelease(impl->pipeline);
    }
    if (impl->bindGroup) {
        // wgpuBindGroupRelease(impl->bindGroup);
    }
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
    
    // Upload vertex data to GPU
    if (!impl->vertexData.empty()) {
        // Release old buffer if it exists
        if (impl->vertexBuffer) {
            wgpuBufferRelease(impl->vertexBuffer);
            impl->vertexBuffer = nullptr;
        }
        
        // Create vertex buffer
        WGPUBufferDescriptor bufferDesc = {};
        bufferDesc.label = "Vertex Buffer";
        bufferDesc.size = impl->vertexData.size();
        bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = true;
        
        impl->vertexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        
        if (impl->vertexBuffer) {
            // Copy data to buffer
            void* mappedData = wgpuBufferGetMappedRange(impl->vertexBuffer, 0, impl->vertexData.size());
            if (mappedData) {
                std::memcpy(mappedData, impl->vertexData.data(), impl->vertexData.size());
                wgpuBufferUnmap(impl->vertexBuffer);
            }
        }
    }
    
    // Upload index data to GPU
    if (impl->indexVector && impl->indexVector->elements() > 0) {
        // Release old buffer if it exists
        if (impl->indexBuffer) {
            wgpuBufferRelease(impl->indexBuffer);
            impl->indexBuffer = nullptr;
        }
        
        // Create index buffer
        const void* indexData = impl->indexVector->data();
        std::size_t indexSize = impl->indexVector->bytes();
        
        WGPUBufferDescriptor bufferDesc = {};
        bufferDesc.label = "Index Buffer";
        bufferDesc.size = indexSize;
        bufferDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = true;
        
        impl->indexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        
        if (impl->indexBuffer) {
            // Copy data to buffer
            void* mappedData = wgpuBufferGetMappedRange(impl->indexBuffer, 0, indexSize);
            if (mappedData) {
                std::memcpy(mappedData, indexData, indexSize);
                wgpuBufferUnmap(impl->indexBuffer);
            }
        }
    }
    
    // Upload textures
    uploadTextures(webgpuUploadPass);
}

void Drawable::uploadTextures(UploadPass&) const noexcept {
    for (const auto& texture : textures) {
        if (texture) {
            texture->upload();
        }
    }
}

void Drawable::draw(PaintParameters& parameters) const {
    if (!getEnabled()) {
        return;
    }
    
    // Get the render pass
    if (!parameters.renderPass) {
        return;
    }
    
    // For now, create a dummy implementation
    // In a complete implementation, we would get this from the RenderPass
    WGPURenderPassEncoder renderPassEncoder = reinterpret_cast<WGPURenderPassEncoder>(1);
    
    // Set the pipeline
    if (impl->pipeline) {
        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, impl->pipeline);
    }
    
    // Bind vertex buffer
    if (impl->vertexBuffer) {
        wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 0, impl->vertexBuffer, 0, impl->vertexData.size());
    }
    
    // Bind index buffer if available
    if (impl->indexBuffer && impl->indexVector) {
        // For now, we assume 16-bit indices
        WGPUIndexFormat indexFormat = WGPUIndexFormat_Uint16;
        wgpuRenderPassEncoderSetIndexBuffer(renderPassEncoder, impl->indexBuffer, indexFormat, 0, impl->indexVector->bytes());
    }
    
    // Bind uniform buffers and textures via bind group
    if (impl->bindGroup) {
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, impl->bindGroup, 0, nullptr);
    }
    
    // Draw
    if (impl->indexBuffer && impl->indexVector) {
        // Draw indexed
        uint32_t indexCount = static_cast<uint32_t>(impl->indexVector->elements());
        wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, indexCount, 1, 0, 0, 0);
    } else if (impl->vertexBuffer && impl->vertexCount > 0) {
        // Draw non-indexed
        uint32_t vertexCount = static_cast<uint32_t>(impl->vertexCount);
        wgpuRenderPassEncoderDraw(renderPassEncoder, vertexCount, 1, 0, 0);
    }
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indices, std::vector<UniqueDrawSegment> segments) {
    impl->indexVector = std::move(indices);
    // Note: DrawSegments are handled differently in WebGPU
    // We'll use the entire index buffer for now
    (void)segments;
    
    // Mark as dirty to rebuild pipeline if needed
    buildWebGPUPipeline();
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType) {
    impl->vertexData = std::move(data);
    impl->vertexCount = count;
    if (count > 0) {
        impl->vertexSize = impl->vertexData.size() / count;
    }
}

const gfx::UniformBufferArray& Drawable::getUniformBuffers() const {
    return *impl->uniformBuffers;
}

gfx::UniformBufferArray& Drawable::mutableUniformBuffers() {
    return *impl->uniformBuffers;
}

void Drawable::setEnableColor(bool value) {
    impl->colorEnabled = value;
}

void Drawable::setColorMode(const gfx::ColorMode& value) {
    impl->colorMode = value;
}

void Drawable::setEnableDepth(bool value) {
    impl->depthEnabled = value;
}

void Drawable::setDepthType(gfx::DepthMaskType value) {
    impl->depthMask = value;
}

void Drawable::setDepthModeFor3D(const gfx::DepthMode& value) {
    // Set depth mode for 3D rendering
    impl->depthEnabled = value.func != gfx::DepthFunctionType::Never;
    impl->depthMask = value.mask;
}

void Drawable::setStencilModeFor3D(const gfx::StencilMode& value) {
    // Stencil state will be configured in the pipeline
    // Store the value for pipeline creation
    (void)value;
}

void Drawable::setLineWidth(int32_t value) {
    // WebGPU doesn't support line width directly
    // This might need to be handled in the shader
}

void Drawable::setCullFaceMode(const gfx::CullFaceMode& value) {
    impl->cullFaceMode = value;
}

void Drawable::updateVertexAttributes(gfx::VertexAttributeArrayPtr attributes,
                                     std::size_t vertexCount,
                                     gfx::DrawMode drawMode,
                                     gfx::IndexVectorBasePtr indices,
                                     const SegmentBase* segments,
                                     std::size_t segmentCount) {
    // Update vertex attributes and rebuild pipeline if needed
    vertexAttributes = std::move(attributes);
    impl->vertexCount = vertexCount;
    // Store draw mode if needed for pipeline creation
    impl->indexVector = std::move(indices);
    
    // Note: Segments are handled differently in WebGPU
    // We'll use the entire buffer for now
    (void)segments;
    (void)segmentCount;
    
    buildWebGPUPipeline();
}

void Drawable::buildWebGPUPipeline() noexcept {
    // Pipeline creation is deferred until we have a shader program
    // The actual pipeline will be created when we bind with a specific shader
    // This is because WebGPU pipelines are immutable and depend on:
    // - Shader modules
    // - Vertex layout
    // - Render state (depth, stencil, blend)
    // For now, we just mark that pipeline needs rebuilding
    if (impl->pipeline) {
        wgpuRenderPipelineRelease(impl->pipeline);
        impl->pipeline = nullptr;
    }
}



} // namespace webgpu
} // namespace mbgl