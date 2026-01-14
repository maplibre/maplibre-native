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
    texturesToBlit.emplace(texHandle, textureToBlit);

    const auto& textureToBlitVK = static_cast<Texture2D*>(textureToBlit.get());
    textureToBlitVK->setSize(imageSize);
    textureToBlitVK->setFormat(texture.get()->getFormat(), gfx::TextureChannelDataType::UnsignedByte);
    textureToBlitVK->setUsage(Texture2DUsage::Blit);
    textureToBlitVK->create();

    std::vector<std::function<void(gfx::Context&)>> deletionQueue;
    context.submitOneTimeCommand(
        [&](const vk::UniqueCommandBuffer& commandBuffer) {
            textureToBlitVK->uploadSubRegion(pixelData, imageSize, 0, 0, commandBuffer, &deletionQueue);
        },
        true);
    for (const auto& function : deletionQueue) function(context);
    deletionQueue.clear();

    gfx::DynamicTexture::uploadImage(pixelData, texHandle);
}

void DynamicTexture::uploadDeferredImages() {
    std::scoped_lock lock(mutex);

    const auto& textureVK = static_cast<Texture2D*>(texture.get());
    for (const auto& pair : texturesToBlit) {
        const auto &rect = pair.first.getRectangle();
        const auto &textureToBlitVK = static_cast<Texture2D *>(pair.second.get());
        textureVK->copyImage(textureToBlitVK->getVulkanImage(), {rect.w, rect.h}, rect.x, rect.y);
    }
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
