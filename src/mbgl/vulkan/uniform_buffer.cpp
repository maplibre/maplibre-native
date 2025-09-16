#include <mbgl/vulkan/uniform_buffer.hpp>

#include <mbgl/vulkan/context.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/render_pass.hpp>

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
      buffer(std::move(other.buffer)) {} // NOLINT(bugprone-use-after-move)

UniformBuffer::~UniformBuffer() {
    buffer.getContext().renderingStats().numUniformBuffers--;
    buffer.getContext().renderingStats().memUniformBuffers -= size;
}

void UniformBuffer::update(const void* data, std::size_t dataSize) {
    assert(dataSize <= size);
    if (dataSize > size || dataSize > buffer.getSizeInBytes()) {
        Log::Error(Event::General,
                   "Mismatched size given to UBO update, expected max " + std::to_string(size) + ", got " +
                       std::to_string(dataSize));
        return;
    }

    buffer.getContext().renderingStats().numUniformUpdates++;
    buffer.getContext().renderingStats().uniformUpdateBytes += dataSize;
    buffer.update(data, dataSize, /*offset=*/0);
}

const std::shared_ptr<gfx::UniformBuffer>& UniformBufferArray::set(const size_t id,
                                                                   std::shared_ptr<gfx::UniformBuffer> uniformBuffer) {
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
    return uniformBufferVector[id];
}

void UniformBufferArray::createOrUpdate(
    const size_t id, const void* data, std::size_t size, gfx::Context& context, bool persistent) {
    if (descriptorSet) {
        if (auto& ubo = get(id); !ubo || ubo->getSize() != size) {
            descriptorSet->markDirty();
        }
    }

    gfx::UniformBufferArray::createOrUpdate(id, data, size, context, persistent);
}

void UniformBufferArray::bind(gfx::RenderPass& renderPass) {
    bindDescriptorSets(static_cast<RenderPass&>(renderPass).getEncoder());
}

void UniformBufferArray::bindDescriptorSets(CommandEncoder& encoder) {
    if (!descriptorSet) {
        descriptorSet = std::make_unique<UniformDescriptorSet>(encoder.getContext(), descriptorSetType);
    }

    descriptorSet->update(*this, descriptorStartIndex, descriptorStorageCount, descriptorUniformCount);

    const auto frameCount = encoder.getContext().getBackend().getMaxFrames();
    const int32_t currentIndex = encoder.getContext().getCurrentFrameResourceIndex();
    const int32_t prevIndex = currentIndex == 0 ? frameCount - 1 : currentIndex - 1;

    for (uint32_t i = 0; i < descriptorStorageCount + descriptorUniformCount; ++i) {
        const uint32_t index = descriptorStartIndex + i;

        if (!uniformBufferVector[index]) {
            continue;
        }

        auto& buff = static_cast<UniformBuffer*>(uniformBufferVector[index].get())->mutableBufferResource();
        buff.updateVulkanBuffer(currentIndex, prevIndex);
    }

    descriptorSet->bind(encoder);
}

} // namespace vulkan
} // namespace mbgl
