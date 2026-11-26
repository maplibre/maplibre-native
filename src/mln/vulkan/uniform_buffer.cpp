#include <mln/vulkan/uniform_buffer.hpp>

#include <mln/vulkan/context.hpp>
#include <mln/util/logging.hpp>
#include <mln/vulkan/command_encoder.hpp>
#include <mln/vulkan/render_pass.hpp>

#include <cassert>

#include <algorithm>

namespace mln {
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
    if (descriptorSetType == DescriptorSetType::Global) {
        auto& boundSet = getDescriptorSetForBuffers(encoder);
        boundSet.update(*this, descriptorStartIndex, descriptorStorageCount, descriptorUniformCount);
        updateBufferResources(encoder);
        boundSet.bind(encoder);
        return;
    }

    if (!descriptorSet) {
        descriptorSet = std::make_unique<UniformDescriptorSet>(encoder.getContext(), descriptorSetType);
    }

    descriptorSet->update(*this, descriptorStartIndex, descriptorStorageCount, descriptorUniformCount);
    updateBufferResources(encoder);
    descriptorSet->bind(encoder);
}

UniformDescriptorSet& UniformBufferArray::getDescriptorSetForBuffers(CommandEncoder& encoder) {
    // Forget the sets of buffers that no longer exist (their deletion is deferred past
    // the frames in flight by the descriptor set itself).
    std::erase_if(bufferSets, [](const BufferSet& entry) {
        return std::ranges::any_of(entry.owners, [](const auto& owner) { return owner.expired(); });
    });

    const uint32_t count = descriptorStorageCount + descriptorUniformCount;
    std::vector<const gfx::UniformBuffer*> buffers(count);
    for (uint32_t i = 0; i < count; ++i) {
        buffers[i] = uniformBufferVector[descriptorStartIndex + i].get();
    }

    for (auto& entry : bufferSets) {
        if (entry.buffers == buffers) {
            return *entry.descriptorSet;
        }
    }

    BufferSet entry;
    entry.buffers = std::move(buffers);
    for (uint32_t i = 0; i < count; ++i) {
        if (entry.buffers[i]) {
            entry.owners.emplace_back(uniformBufferVector[descriptorStartIndex + i]);
        }
    }
    entry.descriptorSet = std::make_unique<UniformDescriptorSet>(encoder.getContext(), descriptorSetType);
    return *bufferSets.emplace_back(std::move(entry)).descriptorSet;
}

void UniformBufferArray::updateBufferResources(CommandEncoder& encoder) {
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
}

} // namespace vulkan
} // namespace mln
