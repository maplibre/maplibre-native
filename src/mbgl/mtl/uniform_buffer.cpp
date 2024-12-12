#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/mtl/render_pass.hpp>
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
    buffer.update(data, size_, /*offset=*/0);
}

void UniformBufferArray::bind(RenderPass& renderPass) const noexcept {
    for (size_t id = 0; id < allocatedSize(); id++) {
        const auto& uniformBuffer = get(id);
        if (!uniformBuffer) continue;
        const auto& buffer = static_cast<UniformBuffer&>(*uniformBuffer.get());
        const auto& resource = buffer.getBufferResource();
        if (buffer.getBindVertex()) {
            renderPass.bindVertex(resource, 0, id);
        }
        if (buffer.getBindFragment()) {
            renderPass.bindFragment(resource, 0, id);
        }
    }
}

} // namespace mtl
} // namespace mbgl
