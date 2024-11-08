#include <mbgl/vulkan/descriptor_set.hpp>

#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <cassert>
#include <math.h>

#define USE_DESCRIPTOR_POOL_RESET

namespace mbgl {
namespace vulkan {

DescriptorSet::DescriptorSet(Context& context_, DescriptorSetType type_)
    : context(context_),
      type(type_) {}

DescriptorSet::~DescriptorSet() {
    context.enqueueDeletion(
        [type_ = type, poolIndex = descriptorPoolIndex, sets = std::move(descriptorSets)](auto& context_) mutable {
            auto& poolInfo = context_.getDescriptorPool(type_).pools[poolIndex];
#ifdef USE_DESCRIPTOR_POOL_RESET
            poolInfo.unusedSets.push(std::move(sets));
#else
            context_.getBackend().getDevice()->freeDescriptorSets(poolInfo.pool.get(), sets);
            poolInfo.remainingSets += sets.size();
#endif
        });
}

void DescriptorSet::createDescriptorPool(DescriptorPoolGrowable& growablePool) {
    const auto& device = context.getBackend().getDevice();

    const uint32_t maxSets = static_cast<uint32_t>(growablePool.maxSets *
                                                   std::pow(growablePool.growFactor, growablePool.pools.size()));
    const vk::DescriptorPoolSize size = {type != DescriptorSetType::DrawableImage
                                             ? vk::DescriptorType::eUniformBuffer
                                             : vk::DescriptorType::eCombinedImageSampler,
                                         maxSets * growablePool.descriptorsPerSet};

#ifdef USE_DESCRIPTOR_POOL_RESET
    const auto poolFlags = vk::DescriptorPoolCreateFlags();
#else
    const auto poolFlags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
#endif

    const auto descriptorPoolInfo = vk::DescriptorPoolCreateInfo(poolFlags).setPoolSizes(size).setMaxSets(maxSets);

    growablePool.pools.emplace_back(device->createDescriptorPoolUnique(descriptorPoolInfo), maxSets);
    growablePool.currentPoolIndex = growablePool.pools.size() - 1;
};

void DescriptorSet::allocate() {
    MLN_TRACE_FUNC();

    if (!descriptorSets.empty()) {
        return;
    }

    const auto& device = context.getBackend().getDevice();
    const auto& descriptorSetLayout = context.getDescriptorSetLayout(type);
    auto& growablePool = context.getDescriptorPool(type);
    const std::vector<vk::DescriptorSetLayout> layouts(context.getBackend().getMaxFrames(), descriptorSetLayout);

    if (growablePool.currentPoolIndex == -1 ||
        (growablePool.current().unusedSets.empty() && growablePool.current().remainingSets < layouts.size())) {
#ifdef USE_DESCRIPTOR_POOL_RESET
        // find a pool that has unused allocated descriptor sets
        const auto& unusedPoolIt = std::find_if(
            growablePool.pools.begin(), growablePool.pools.end(), [&](const auto& p) { return !p.unusedSets.empty(); });

        if (unusedPoolIt != growablePool.pools.end()) {
            growablePool.currentPoolIndex = std::distance(growablePool.pools.begin(), unusedPoolIt);
        } else
#endif
        {
            // find a pool that has available memory to allocate more descriptor sets
            const auto& freePoolIt = std::find_if(growablePool.pools.begin(),
                                                  growablePool.pools.end(),
                                                  [&](const auto& p) { return p.remainingSets >= layouts.size(); });

            if (freePoolIt != growablePool.pools.end()) {
                growablePool.currentPoolIndex = std::distance(growablePool.pools.begin(), freePoolIt);
            } else {
                createDescriptorPool(growablePool);
            }
        }
    }

    descriptorPoolIndex = growablePool.currentPoolIndex;

#ifdef USE_DESCRIPTOR_POOL_RESET
    if (!growablePool.current().unusedSets.empty()) {
        descriptorSets = growablePool.current().unusedSets.front();
        growablePool.current().unusedSets.pop();
    } else
#endif
    {
        descriptorSets = device->allocateDescriptorSets(vk::DescriptorSetAllocateInfo()
                                                            .setDescriptorPool(growablePool.current().pool.get())
                                                            .setSetLayouts(layouts));
        growablePool.current().remainingSets -= descriptorSets.size();
    }

    dirty = std::vector(descriptorSets.size(), true);
}

void DescriptorSet::markDirty(bool value) {
    std::fill(dirty.begin(), dirty.end(), value);
}

void DescriptorSet::bind(CommandEncoder& encoder) {
    MLN_TRACE_FUNC();
    auto& commandBuffer = encoder.getCommandBuffer();

    const uint8_t index = context.getCurrentFrameResourceIndex();

    commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                      context.getGeneralPipelineLayout().get(),
                                      static_cast<uint32_t>(type),
                                      descriptorSets[index],
                                      nullptr);
}

UniformDescriptorSet::UniformDescriptorSet(Context& context_, DescriptorSetType type_)
    : DescriptorSet(context_, type_) {}

void UniformDescriptorSet::update(const gfx::UniformBufferArray& uniforms,
                                  uint32_t uniformStartIndex,
                                  uint32_t descriptorBindingCount) {
    MLN_TRACE_FUNC();

    allocate();

    const uint8_t frameIndex = context.getCurrentFrameResourceIndex();
    if (!dirty[frameIndex]) {
        return;
    }

    const auto& device = context.getBackend().getDevice();

    for (size_t index = 0; index < descriptorBindingCount; ++index) {
        vk::DescriptorBufferInfo descriptorBufferInfo;

        if (const auto& uniformBuffer = uniforms.get(uniformStartIndex + index)) {
            const auto& uniformBufferImpl = static_cast<const UniformBuffer&>(*uniformBuffer);
            const auto& bufferResource = uniformBufferImpl.getBufferResource();
            descriptorBufferInfo.setBuffer(bufferResource.getVulkanBuffer())
                .setOffset(bufferResource.getVulkanBufferOffset())
                .setRange(bufferResource.getSizeInBytes());
        } else {
            descriptorBufferInfo.setBuffer(context.getDummyUniformBuffer()->getVulkanBuffer())
                .setOffset(0)
                .setRange(VK_WHOLE_SIZE);
        }

        const auto writeDescriptorSet = vk::WriteDescriptorSet()
                                            .setBufferInfo(descriptorBufferInfo)
                                            .setDescriptorCount(1)
                                            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                            .setDstBinding(index)
                                            .setDstSet(descriptorSets[frameIndex]);

        device->updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    dirty[frameIndex] = false;
}

ImageDescriptorSet::ImageDescriptorSet(Context& context_)
    : DescriptorSet(context_, DescriptorSetType::DrawableImage) {}

void ImageDescriptorSet::update(const std::array<gfx::Texture2DPtr, shaders::maxTextureCountPerShader>& textures) {
    MLN_TRACE_FUNC();
    allocate();

    const uint8_t frameIndex = context.getCurrentFrameResourceIndex();
    if (!dirty[frameIndex]) {
        return;
    }

    const auto& device = context.getBackend().getDevice();

    for (size_t id = 0; id < shaders::maxTextureCountPerShader; ++id) {
        const auto& texture = id < textures.size() ? textures[id] : nullptr;
        auto& textureImpl = texture ? static_cast<Texture2D&>(*texture) : *context.getDummyTexture();

        const auto descriptorImageInfo = vk::DescriptorImageInfo()
                                             .setImageLayout(textureImpl.getVulkanImageLayout())
                                             .setImageView(textureImpl.getVulkanImageView().get())
                                             .setSampler(textureImpl.getVulkanSampler());

        const auto writeDescriptorSet = vk::WriteDescriptorSet()
                                            .setImageInfo(descriptorImageInfo)
                                            .setDescriptorCount(1)
                                            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                                            .setDstBinding(id)
                                            .setDstSet(descriptorSets[frameIndex]);

        device->updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    dirty[frameIndex] = false;
}

} // namespace vulkan
} // namespace mbgl
