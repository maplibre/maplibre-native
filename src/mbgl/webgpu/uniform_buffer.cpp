#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/logging.hpp>
#include <cstring>

namespace mbgl {
namespace webgpu {

UniformBuffer::UniformBuffer(Context& context_, const void* data, std::size_t size_, bool persistent_)
    : context(context_),
      size(size_),
      persistent(persistent_) {
    
    if (size == 0) {
        Log::Error(Event::Render, "Cannot create uniform buffer with size 0");
        return;
    }
    
    // Align size to 16 bytes (WebGPU requirement for uniform buffers)
    size = (size + 15) & ~15;
    
    createBuffer(data);
}

UniformBuffer::~UniformBuffer() {
    if (buffer) {
        // wgpuBufferDestroy(buffer);
        // wgpuBufferRelease(buffer);
    }
}

void UniformBuffer::createBuffer(const void* data) {
    auto* impl = context.getImpl();
    if (!impl) {
        return;
    }
    
    WGPUDevice device = impl->getDevice();
    if (!device) {
        return;
    }
    
    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.label = "Uniform Buffer";
    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    
    if (data) {
        // Create buffer with initial data
        bufferDesc.mappedAtCreation = true;
        // buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        
        if (buffer) {
            // void* mappedData = wgpuBufferGetMappedRange(buffer, 0, size);
            // if (mappedData) {
            //     std::memcpy(mappedData, data, size);
            //     wgpuBufferUnmap(buffer);
            // }
        }
    } else {
        // Create buffer without initial data
        bufferDesc.mappedAtCreation = false;
        // buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
    }
    
    if (!buffer) {
        Log::Error(Event::Render, "Failed to create WebGPU uniform buffer");
    }
}

void UniformBuffer::update(const void* data, std::size_t updateSize) {
    if (!buffer || !data || updateSize == 0) {
        return;
    }
    
    if (updateSize > size) {
        Log::Warning(Event::Render, "Update size exceeds buffer size");
        updateSize = size;
    }
    
    auto* impl = context.getImpl();
    if (!impl) {
        return;
    }
    
    WGPUQueue queue = impl->getQueue();
    if (!queue) {
        return;
    }
    
    // Update buffer data via queue
    // wgpuQueueWriteBuffer(queue, buffer, 0, data, updateSize);
}

WGPUBindGroupEntry UniformBuffer::createBindGroupEntry(uint32_t binding) const {
    WGPUBindGroupEntry entry = {};
    entry.binding = binding;
    entry.buffer = buffer;
    entry.offset = 0;
    entry.size = size;
    return entry;
}

// StorageBuffer implementation
StorageBuffer::StorageBuffer(Context& context, const void* data, std::size_t size, bool persistent)
    : UniformBuffer(context, nullptr, size, persistent) {
    
    // Override buffer creation with storage buffer usage
    auto* impl = context.getImpl();
    if (!impl) {
        return;
    }
    
    WGPUDevice device = impl->getDevice();
    if (!device) {
        return;
    }
    
    // Storage buffers don't need 16-byte alignment like uniform buffers
    this->size = size;
    
    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.label = "Storage Buffer";
    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst;
    
    if (data) {
        bufferDesc.mappedAtCreation = true;
        // buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        
        // if (buffer) {
        //     void* mappedData = wgpuBufferGetMappedRange(buffer, 0, size);
        //     if (mappedData) {
        //         std::memcpy(mappedData, data, size);
        //         wgpuBufferUnmap(buffer);
        //     }
        // }
    } else {
        bufferDesc.mappedAtCreation = false;
        // buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
    }
}

WGPUBindGroupEntry StorageBuffer::createBindGroupEntry(uint32_t binding) const {
    WGPUBindGroupEntry entry = {};
    entry.binding = binding;
    entry.buffer = getBuffer();
    entry.offset = 0;
    entry.size = getSize();
    // Note: The binding type in the layout should be set to Storage, not Uniform
    return entry;
}

} // namespace webgpu
} // namespace mbgl