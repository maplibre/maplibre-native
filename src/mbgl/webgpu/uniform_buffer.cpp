#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/util/logging.hpp>
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

UniformBuffer UniformBuffer::clone() const {
    UniformBuffer newBuffer(context, nullptr, getSize());

    // Note: We can't directly copy buffer contents in WebGPU without a command encoder
    // The data will need to be copied when updated

    return newBuffer;
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
    bindWebgpu(static_cast<RenderPass&>(renderPass));
}

// The copy() method is now implemented inline in the header file to match Metal

void UniformBufferArray::bindWebgpu(RenderPass& renderPass) const noexcept {
    // In WebGPU, uniform buffers are bound through bind groups per drawable,
    // not globally to the render pass like in Metal.
    // The actual binding happens in Drawable::draw when creating the bind group.
    // This method is here for API compatibility with Metal.
}

} // namespace webgpu
} // namespace mbgl