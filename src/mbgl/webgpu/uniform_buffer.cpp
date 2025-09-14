#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <cstring>

namespace mbgl {
namespace webgpu {

UniformBuffer::UniformBuffer(Context& context_, const void* data, std::size_t size_)
    : gfx::UniformBuffer(size_), 
      context(context_) {
    
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    
    if (device && size > 0) {
        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView label = {"Uniform Buffer", strlen("Uniform Buffer")};
        bufferDesc.label = label;
        bufferDesc.size = size;
        bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = data ? 1 : 0;
        
        buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        
        if (buffer && data) {
            void* mappedData = wgpuBufferGetMappedRange(buffer, 0, size);
            if (mappedData) {
                std::memcpy(mappedData, data, size);
                wgpuBufferUnmap(buffer);
            }
        }
    }
}

UniformBuffer::UniformBuffer(const UniformBuffer& other)
    : gfx::UniformBuffer(other.getSize()),
      context(other.context) {
    
    if (other.buffer && getSize() > 0) {
        // Create a new buffer with the same size
        auto& backend = static_cast<RendererBackend&>(context.getBackend());
        WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
        
        if (device) {
            WGPUBufferDescriptor bufferDesc = {};
            WGPUStringView label = {"Uniform Buffer (Copy)", strlen("Uniform Buffer (Copy)")};
            bufferDesc.label = label;
            bufferDesc.size = getSize();
            bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
            bufferDesc.mappedAtCreation = 0;
            buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
            
            // Note: We can't directly copy buffer contents in WebGPU without a command encoder
            // The data will need to be copied when updated
        }
    }
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : gfx::UniformBuffer(std::move(other)),
      context(other.context),
      buffer(other.buffer) {
    other.buffer = nullptr;
}

UniformBuffer::~UniformBuffer() {
    if (buffer) {
        wgpuBufferRelease(buffer);
        buffer = nullptr;
    }
}

void UniformBuffer::update(const void* data, std::size_t dataSize) {
    if (data && dataSize > 0 && buffer) {
        auto& backend = static_cast<RendererBackend&>(context.getBackend());
        WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());
        if (queue) {
            wgpuQueueWriteBuffer(queue, buffer, 0, data, dataSize);
        }
    }
}

// UniformBufferArray implementation

void UniformBufferArray::bind(gfx::RenderPass& renderPass) {
    // Note: In WebGPU, uniform buffers are bound through bind groups
    // which are created at the drawable level with the pipeline.
    // This method primarily ensures buffers are ready for use.
    // The actual binding happens in Drawable::draw() via bind groups.
    (void)renderPass;
}

gfx::UniqueUniformBuffer UniformBufferArray::copy(const gfx::UniformBuffer& buffer) {
    // Since we know this is a WebGPU context, we can safely cast
    auto* webgpuBuffer = static_cast<const UniformBuffer*>(&buffer);
    return std::make_unique<UniformBuffer>(*webgpuBuffer);
}

} // namespace webgpu
} // namespace mbgl