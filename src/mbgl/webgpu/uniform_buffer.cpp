#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <cstring>

namespace mbgl {
namespace webgpu {

UniformBuffer::UniformBuffer(const void* data, std::size_t size, bool /*persistent*/)
    : gfx::UniformBuffer(size), size_(size) {
    
    data_.resize(size);
    if (data) {
        std::memcpy(data_.data(), data, size);
    }
    
    // TODO: Create actual WebGPU buffer
    // buffer = device.CreateBuffer(&bufferDesc);
}

UniformBuffer::~UniformBuffer() {
    // TODO: Release WebGPU buffer
}

void UniformBuffer::update(const void* data, std::size_t size) {
    if (size != size_) {
        // Resize if needed
        data_.resize(size);
        size_ = size;
    }
    
    if (data) {
        std::memcpy(data_.data(), data, size);
    }
    
    // TODO: Update WebGPU buffer
    // queue.WriteBuffer(buffer, 0, data, size);
}

} // namespace webgpu
} // namespace mbgl