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

    std::vector<std::function<void(gfx::Context&)>> deletionQueue;
    const auto& textureVK = static_cast<Texture2D*>(texture.get());
    context.submitOneTimeCommand([&](const vk::UniqueCommandBuffer& commandBuffer) {
        textureVK->uploadSubRegion(pixelData, imageSize, rect.x, rect.y, commandBuffer, &deletionQueue);
    }, true);
    for (const auto& function : deletionQueue) function(context);
    deletionQueue.clear();

    gfx::DynamicTexture::uploadImage(pixelData, texHandle);
}

} // namespace vulkan
} // namespace mbgl
