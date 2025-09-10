#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <cstring>

namespace mbgl {
namespace webgpu {

UniformBuffer::UniformBuffer(Context& context_, const void* data, std::size_t size)
    : gfx::UniformBuffer(size), 
      context(context_) {
    
    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    
    if (device && size > 0) {
        WGPUBufferDescriptor bufferDesc = {};
        bufferDesc.label = "Uniform Buffer";
        bufferDesc.size = size;
        bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = (data != nullptr);
        
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
            bufferDesc.label = "Uniform Buffer (Copy)";
            bufferDesc.size = getSize();
            bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
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
        auto queue = context.getQueue();
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
    if (auto* webgpuBuffer = dynamic_cast<const UniformBuffer*>(&buffer)) {
        return std::make_unique<UniformBuffer>(*webgpuBuffer);
    }
    return nullptr;
}

} // namespace webgpu
} // namespace mbgl