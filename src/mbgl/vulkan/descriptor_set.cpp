#include <mbgl/vulkan/descriptor_set.hpp>

#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/texture2d.hpp>

#include <cassert>

namespace mbgl {
namespace vulkan {

DescriptorSet::DescriptorSet(Context& context_, DescriptorSetType type_)
    : context(context_),
      type(type_) {}

DescriptorSet::~DescriptorSet() {
    context.enqueueDeletion([sets = std::move(descriptorSets)](const auto& context_) mutable {
        context_.getBackend().getDevice()->freeDescriptorSets(context_.getGlobalDescriptorPool().get(), sets);
    });
}

void DescriptorSet::allocate() {
    if (!descriptorSets.empty()) {
        return;
    }

    const auto& device = context.getBackend().getDevice();
    const auto& descriptorPool = context.getGlobalDescriptorPool();
    const auto& descriptorSetLayout = context.getDescriptorSetLayout(type);
    const std::vector<vk::DescriptorSetLayout> layouts(context.getBackend().getMaxFrames(), descriptorSetLayout);

    const auto descriptorAllocInfo =
        vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setSetLayouts(layouts);

    descriptorSets = device->allocateDescriptorSets(descriptorAllocInfo);

    markDirty(true);
}

void DescriptorSet::markDirty(bool value) {
    dirty = std::vector(descriptorSets.size(), value);
}

void DescriptorSet::bind(CommandEncoder& encoder) {
    auto& commandBuffer = encoder.getCommandBuffer();

    const uint8_t index = context.getCurrentFrameResourceIndex();

    commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                      context.getGeneralPipelineLayout().get(),
                                      static_cast<uint32_t>(type),
                                      descriptorSets[index],
                                      nullptr);
}

UniformDescriptorSet::UniformDescriptorSet(Context& context_, DescriptorSetType type_)
    : DescriptorSet(context_, type_) {

}

void UniformDescriptorSet::update(const gfx::UniformBufferArray& uniforms,
                                  uint32_t uniformStartIndex,
                                  uint32_t descriptorBindingCount) {

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
}

ImageDescriptorSet::ImageDescriptorSet(Context& context_)
    : DescriptorSet(context_, DescriptorSetType::DrawableImage) {

}

void ImageDescriptorSet::update(const std::array<gfx::Texture2DPtr, shaders::maxTextureCountPerShader>& textures) {

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
}

} // namespace vulkan
} // namespace mbgl
