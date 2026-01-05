#include <mbgl/gl/dynamic_texture.hpp>
#include <mbgl/gl/context.hpp>

namespace mbgl {
namespace gl {

DynamicTexture::DynamicTexture(Context& context, Size size, gfx::TexturePixelType pixelType)
    : gfx::DynamicTexture(context, size, pixelType) {
    deferredCreation = true;
}

void DynamicTexture::uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) {
    std::scoped_lock lock(mutex);
    const auto& rect = texHandle.getRectangle();
    const auto imageSize = Size(rect.w, rect.h);

    auto size = imageSize.area() * texture->getPixelStride();
    auto imageData = std::make_unique<uint8_t[]>(size);
    std::copy(pixelData, pixelData + size, imageData.get());
    imagesToUpload.emplace(texHandle, std::move(imageData));

    gfx::DynamicTexture::uploadImage(pixelData, texHandle);
}

void DynamicTexture::uploadDeferredImages() {
    std::scoped_lock lock(mutex);
    if (deferredCreation) {
        texture->create();
        deferredCreation = false;
    }
    for (const auto& pair : imagesToUpload) {
        const auto& rect = pair.first.getRectangle();
        texture->uploadSubRegion(pair.second.get(), Size(rect.w, rect.h), rect.x, rect.y);
    }
    imagesToUpload.clear();
}

bool DynamicTexture::removeTexture(const gfx::TextureHandle& texHandle) {
    if (gfx::DynamicTexture::removeTexture(texHandle)) {
        std::scoped_lock lock(mutex);
        imagesToUpload.erase(texHandle);
        return true;
    }
    return false;
}

} // namespace gl
} // namespace mbgl
