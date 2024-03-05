#include <mbgl/mtl/uniform_buffer.hpp>

#include <mbgl/mtl/context.hpp>
#include <mbgl/util/logging.hpp>

#include <cassert>

namespace mbgl {
namespace mtl {

UniformBuffer::UniformBuffer(BufferResource&& buffer_)
    : gfx::UniformBuffer(buffer_.getSizeInBytes()),
      buffer(std::move(buffer_)) {
    buffer.getContext().renderingStats().numUniformBuffers++;
    buffer.getContext().renderingStats().memUniformBuffers += size;
}

UniformBuffer::UniformBuffer(UniformBuffer&& other)
    : gfx::UniformBuffer(std::move(other)),
      buffer(std::move(other.buffer)) {}

UniformBuffer::~UniformBuffer() {
    buffer.getContext().renderingStats().numUniformBuffers--;
    buffer.getContext().renderingStats().memUniformBuffers -= size;
}

void UniformBuffer::update(const void* data, std::size_t size_) {
    /*assert(size == size_);
    if (size != size_ || size != buffer.getSizeInBytes()) {
        Log::Error(
            Event::General,
            "Mismatched size given to UBO update, expected " + std::to_string(size) + ", got " + std::to_string(size_));
        return;
    }*/

    buffer.getContext().renderingStats().numUniformUpdates++;
    buffer.getContext().renderingStats().uniformUpdateBytes += size_;
    buffer.update(data, size, /*offset=*/0);
}

} // namespace mtl
} // namespace mbgl
