#include <mbgl/webgpu/buffer_resource.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>

#include <webgpu/webgpu_cpp.h>

#include <cstring>

namespace mbgl {
namespace webgpu {

BufferResource::BufferResource(Context& context_,
                               const void* data,
                               std::size_t size_,
                               wgpu::BufferUsage usage_,
                               bool persistent_)
    : context(context_), size(size_), usage(usage_), persistent(persistent_) {
    
    if (size == 0) {
        return;
    }

    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    wgpu::Device device = backend.getDevice();
    
    if (!device) {
        return;
    }

    // Create buffer descriptor
    wgpu::BufferDescriptor bufferDesc{};
    bufferDesc.label = "Buffer Resource";
    bufferDesc.usage = usage | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = size;
    bufferDesc.mappedAtCreation = (data != nullptr);

    // Create buffer
    buffer = device.CreateBuffer(&bufferDesc);
    
    if (buffer && data) {
        // Copy initial data if provided
        void* mappedData = buffer.GetMappedRange();
        if (mappedData) {
            std::memcpy(mappedData, data, size);
            buffer.Unmap();
        }
    }
}

BufferResource::~BufferResource() {
    if (buffer) {
        buffer.Destroy();
        buffer = nullptr;
    }
}

void BufferResource::update(const void* data, std::size_t updateSize, std::size_t offset) {
    if (!buffer || !data || updateSize == 0) {
        return;
    }

    auto& backend = static_cast<RendererBackend&>(context.getBackend());
    wgpu::Device device = backend.getDevice();
    wgpu::Queue queue = backend.getQueue();
    
    if (!device || !queue) {
        return;
    }

    // For small updates, use WriteBuffer directly
    if (updateSize <= 65536) { // 64KB threshold
        queue.WriteBuffer(buffer, offset, data, updateSize);
    } else {
        // For larger updates, use a staging buffer
        wgpu::BufferDescriptor stagingDesc{};
        stagingDesc.label = "Staging Buffer";
        stagingDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        stagingDesc.size = updateSize;
        stagingDesc.mappedAtCreation = true;

        wgpu::Buffer stagingBuffer = device.CreateBuffer(&stagingDesc);
        if (stagingBuffer) {
            void* mappedData = stagingBuffer.GetMappedRange();
            if (mappedData) {
                std::memcpy(mappedData, data, updateSize);
                stagingBuffer.Unmap();

                // Copy from staging buffer to destination
                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                encoder.CopyBufferToBuffer(stagingBuffer, 0, buffer, offset, updateSize);
                
                wgpu::CommandBuffer commands = encoder.Finish();
                queue.Submit(1, &commands);
                
                // Staging buffer will be destroyed when it goes out of scope
                stagingBuffer.Destroy();
            }
        }
    }
}

} // namespace webgpu
} // namespace mbgl