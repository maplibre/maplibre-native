#include <mbgl/vulkan/dynamic_texture.hpp>
#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/vulkan/context.hpp>

namespace mbgl {
namespace vulkan {

DynamicTexture::DynamicTexture(Context& context_, Size size, gfx::TexturePixelType pixelType)
    : gfx::DynamicTexture(context_, size, pixelType),
      context(context_) {
    texture->create();
}

void DynamicTexture::uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) {
    std::scoped_lock lock(mutex);
    const auto& rect = texHandle.getRectangle();
    const auto imageSize = Size(rect.w, rect.h);

    gfx::Texture2DPtr textureToBlit = context.createTexture2D();
    const auto& textureToBlitVK = static_cast<Texture2D*>(textureToBlit.get());
    textureToBlitVK->setSize(imageSize);
    textureToBlitVK->setFormat(texture.get()->getFormat(), gfx::TextureChannelDataType::UnsignedByte);
    textureToBlitVK->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                              .wrapU = gfx::TextureWrapType::Clamp,
                                              .wrapV = gfx::TextureWrapType::Clamp});
    textureToBlitVK->setUsage(Texture2DUsage::Blit);
    textureToBlitVK->create();
    texturesToBlit.emplace(texHandle, textureToBlit);

    std::vector<std::function<void(gfx::Context&)>> deletionQueue;
    context.submitOneTimeCommand(
        [&](const vk::UniqueCommandBuffer& commandBuffer) {
            textureToBlitVK->uploadSubRegion(pixelData, imageSize, 0, 0, commandBuffer, &deletionQueue);

            const auto barrier = vk::ImageMemoryBarrier()
                                     .setImage(textureToBlitVK->getVulkanImage())
                                     .setOldLayout(textureToBlitVK->getVulkanImageLayout())
                                     .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                                     .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                                     .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                                     .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                                     .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                                     .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

            commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                                           vk::PipelineStageFlagBits::eTransfer,
                                           {},
                                           nullptr,
                                           nullptr,
                                           barrier,
                                           context.getBackend().getDispatcher());
        },
        true);
    for (const auto& function : deletionQueue) function(context);
    deletionQueue.clear();

    gfx::DynamicTexture::uploadImage(pixelData, texHandle);
}

void DynamicTexture::uploadDeferredImages() {
    std::scoped_lock lock(mutex);

    const auto& textureVK = static_cast<Texture2D*>(texture.get());
    context.submitOneTimeCommand([&](const vk::UniqueCommandBuffer& commandBuffer) {
        textureVK->transitionToTransferLayout(commandBuffer);

        for (const auto& pair : texturesToBlit) {
            const auto& rect = pair.first.getRectangle();
            const auto& textureToBlitVK = static_cast<Texture2D*>(pair.second.get());

            const auto copyInfo = vk::ImageCopy()
                                      .setSrcSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                                      .setDstSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                                      .setDstOffset({rect.x, rect.y, 0})
                                      .setExtent({rect.w, rect.h, 1});

            commandBuffer->copyImage(textureToBlitVK->getVulkanImage(),
                                     vk::ImageLayout::eTransferSrcOptimal,
                                     textureVK->getVulkanImage(),
                                     vk::ImageLayout::eTransferDstOptimal,
                                     copyInfo,
                                     context.getBackend().getDispatcher());
        }

        textureVK->transitionToGeneralLayout(commandBuffer);
    });

    texturesToBlit.clear();
}

bool DynamicTexture::removeTexture(const gfx::TextureHandle& texHandle) {
    if (gfx::DynamicTexture::removeTexture(texHandle)) {
        std::scoped_lock lock(mutex);
        texturesToBlit.erase(texHandle);
        return true;
    }
    return false;
}

} // namespace vulkan
} // namespace mbgl
