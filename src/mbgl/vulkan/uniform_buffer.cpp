#include <mbgl/vulkan/uniform_buffer.hpp>

#include <mbgl/vulkan/context.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/vulkan/command_encoder.hpp>

#include <cassert>

namespace mbgl {
namespace vulkan {

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
    assert(size == size_);
    if (size != size_ || size != buffer.getSizeInBytes()) {
        Log::Error(
            Event::General,
            "Mismatched size given to UBO update, expected " + std::to_string(size) + ", got " + std::to_string(size_));
        return;
    }

    buffer.getContext().renderingStats().numUniformUpdates++;
    buffer.getContext().renderingStats().uniformUpdateBytes += size_;
    buffer.update(data, size, /*offset=*/0);
}

const std::shared_ptr<gfx::UniformBuffer>& UniformBufferArray::set(const size_t id,
                                                                   std::shared_ptr<gfx::UniformBuffer> uniformBuffer,
                                                                   bool bindVertex,
                                                                   bool bindFragment) {
    if (id >= uniformBufferVector.size()) {
        return nullref;
    }

    if (uniformBufferVector[id] == uniformBuffer) {
        return uniformBufferVector[id];
    }

    if (descriptorSet) {
        descriptorSet->markDirty();
    }

    uniformBufferVector[id] = std::move(uniformBuffer);
    if (uniformBufferVector[id]) {
        uniformBufferVector[id]->setBindVertex(bindVertex);
        uniformBufferVector[id]->setBindFragment(bindFragment);
    }
    return uniformBufferVector[id];
}

void UniformBufferArray::createOrUpdate(
    const size_t id, const void* data, std::size_t size, gfx::Context& context, bool bindVertex, bool bindFragment) {
    if (descriptorSet) {
        if (auto& ubo = get(id); !ubo || ubo->getSize() != size) {
            descriptorSet->markDirty();
        }
    }

    gfx::UniformBufferArray::createOrUpdate(id, data, size, context, bindVertex, bindFragment);
}

void UniformBufferArray::bindDescriptorSets(CommandEncoder& encoder) {
    if (!descriptorSet) {
        descriptorSet = std::make_unique<UniformDescriptorSet>(encoder.getContext(), descriptorSetType);
    }

    descriptorSet->update(*this, descriptorStartIndex, descriptorBindingCount);
    descriptorSet->bind(encoder);
}

} // namespace vulkan
} // namespace mbgl
