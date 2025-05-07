#include <mbgl/gfx/dynamic_texture.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {
namespace gfx {

DynamicTexture::DynamicTexture(Context& context, Size size, TexturePixelType pixelType) {
    mapbox::ShelfPack::ShelfPackOptions options;
    options.autoResize = false;
    shelfPack = mapbox::ShelfPack(size.width, size.height, options);

    texture = context.createTexture2D();
    texture->setSize(size);
    texture->setFormat(pixelType, TextureChannelDataType::UnsignedByte);
    texture->setSamplerConfiguration(
        {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
#if MLN_DEFER_UPLOAD_ON_RENDER_THREAD
    deferredCreation = true;
#else
    texture->create();
#endif
}

const Texture2DPtr& DynamicTexture::getTexture() const {
    assert(texture);
    return texture;
}

TexturePixelType DynamicTexture::getPixelFormat() const {
    assert(texture);
    return texture->getFormat();
}

bool DynamicTexture::isEmpty() const {
    return (numTextures == 0);
}

std::optional<TextureHandle> DynamicTexture::reserveSize(const Size& size, int32_t uniqueId) {
    std::lock_guard<std::mutex> lock(mutex);
    mapbox::Bin* bin = shelfPack.packOne(uniqueId, size.width, size.height);
    if (!bin) {
        return std::nullopt;
    }
    if (bin->refcount() == 1) {
        numTextures++;
    }
    return TextureHandle(*bin);
}

void DynamicTexture::uploadImage(const uint8_t* pixelData, TextureHandle& texHandle) {
    std::lock_guard<std::mutex> lock(mutex);
    const auto& rect = texHandle.getRectangle();
    const auto imageSize = Size(rect.w, rect.h);

#if MLN_DEFER_UPLOAD_ON_RENDER_THREAD
    auto size = imageSize.area() * texture->getPixelStride();
    auto imageData = std::make_unique<uint8_t[]>(size);
    std::copy(pixelData, pixelData + size, imageData.get());
    imagesToUpload.emplace(texHandle, std::move(imageData));
#else
    texture->uploadSubRegion(pixelData, imageSize, rect.x, rect.y);
#endif
    texHandle.needsUpload = false;
}

std::optional<TextureHandle> DynamicTexture::addImage(const uint8_t* pixelData,
                                                      const Size& imageSize,
                                                      int32_t uniqueId) {
    auto texHandle = reserveSize(imageSize, uniqueId);
    if (texHandle && texHandle->isUploadNeeded()) {
        uploadImage(pixelData, *texHandle);
    }
    return texHandle;
}

void DynamicTexture::uploadDeferredImages() {
    std::lock_guard<std::mutex> lock(mutex);
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

void DynamicTexture::removeTexture(const TextureHandle& texHandle) {
    std::lock_guard<std::mutex> lock(mutex);
    auto* bin = shelfPack.getBin(texHandle.getId());
    if (!bin) {
        return;
    }
    auto refcount = shelfPack.unref(*bin);
    if (refcount == 0) {
        numTextures--;
        imagesToUpload.erase(texHandle);
    }
}

} // namespace gfx
} // namespace mbgl
