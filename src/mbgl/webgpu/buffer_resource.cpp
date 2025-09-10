#include <mbgl/webgpu/buffer_resource.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>

#include <cstring>

namespace mbgl {
namespace webgpu {

BufferResource::BufferResource(Context& context_,
                               const void* data,
                               std::size_t size_,
                               uint32_t usage_,
                               bool persistent_)
    : context(&context_), size(size_), usage(usage_), persistent(persistent_) {
    
    if (size == 0) {
        return;
    }

    auto& backend = static_cast<RendererBackend&>(context->getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    
    if (!device) {
        return;
    }

    // Create buffer descriptor
    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.label = "Buffer Resource";
    bufferDesc.usage = usage | WGPUBufferUsage_CopyDst;
    bufferDesc.size = size;
    bufferDesc.mappedAtCreation = (data != nullptr);

    // Create buffer
    buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
    
    if (buffer && data) {
        // Copy initial data if provided
        void* mappedData = wgpuBufferGetMappedRange(buffer, 0, size);
        if (mappedData) {
            std::memcpy(mappedData, data, size);
            wgpuBufferUnmap(buffer);
        }
    }
}

BufferResource::~BufferResource() {
    if (buffer) {
        wgpuBufferRelease(buffer);
        buffer = nullptr;
    }
}

void BufferResource::update(const void* data, std::size_t updateSize, std::size_t offset) {
    if (!buffer || !data || updateSize == 0) {
        return;
    }

    // TODO: Implement buffer update once we have queue access
    // For now, this is a placeholder
    // The proper implementation would use wgpuQueueWriteBuffer
}

} // namespace webgpu
} // namespace mbgl