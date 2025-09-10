#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <cstring>

namespace mbgl {
namespace webgpu {

UniformBuffer::UniformBuffer(Context& context_, const void* data, std::size_t size)
    : gfx::UniformBuffer(size), 
      context(context_) {
    
    // TODO: Create actual WebGPU buffer
    // WGPUBufferDescriptor bufferDesc = {};
    // bufferDesc.size = size;
    // bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    // buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
    
    if (data) {
        update(data, size);
    }
}

UniformBuffer::UniformBuffer(const UniformBuffer& other)
    : gfx::UniformBuffer(other.getSize()),
      context(other.context) {
    // TODO: Copy buffer data
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : gfx::UniformBuffer(std::move(other)),
      context(other.context),
      buffer(other.buffer) {
    other.buffer = nullptr;
}

UniformBuffer::~UniformBuffer() {
    if (buffer) {
        // TODO: Release WebGPU buffer
        // wgpuBufferDestroy(buffer);
        // wgpuBufferRelease(buffer);
        buffer = nullptr;
    }
}

void UniformBuffer::update(const void* data, std::size_t dataSize) {
    if (data && dataSize > 0) {
        // TODO: Update WebGPU buffer
        // wgpuQueueWriteBuffer(queue, buffer, 0, data, dataSize);
    }
}

// UniformBufferArray implementation

void UniformBufferArray::bind(gfx::RenderPass&) {
    // TODO: Bind uniform buffers to WebGPU render pass
    // auto& webgpuRenderPass = static_cast<webgpu::RenderPass&>(renderPass);
    // Iterate through uniform buffers and bind them
    for (const auto& buffer : uniformBufferVector) {
        if (buffer) {
            // TODO: Bind this uniform buffer to the pipeline
        }
    }
}

gfx::UniqueUniformBuffer UniformBufferArray::copy(const gfx::UniformBuffer&) {
    // Create a copy of the uniform buffer
    // For now, return a nullptr since we need a context to create a new buffer
    // This should be properly implemented when we have access to the context
    return nullptr;
}

} // namespace webgpu
} // namespace mbgl