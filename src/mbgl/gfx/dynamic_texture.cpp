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
    texture->create();
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
    mapbox::Bin* bin = shelfPack.packOne(uniqueId, size.width + 2, size.height + 2);
    if (!bin) {
        return std::nullopt;
    }
    if (bin->refcount() == 1) {
        numTextures ++;
    }
    return TextureHandle(*bin);
}

void DynamicTexture::uploadImage(const uint8_t* pixelData, const TextureHandle& texHandle) {
    auto* bin = shelfPack.getBin(texHandle.getId());
    if (!bin || bin->refcount() > 1) {
        return;
    }
    const auto& rect = texHandle.getRectangle();
    const auto imageSize = Size(rect.w - 2, rect.h - 2);

#if MLN_DEFER_UPLOAD_ON_RENDER_THREAD
    auto size = imageSize.area() * texture->getPixelStride();
    auto imageData = std::make_unique<uint8_t[]>(size);
    std::copy(pixelData, pixelData + size, imageData.get());
    imagesToUpload.emplace(texHandle, std::move(imageData));
#else
    texture->uploadSubRegion(pixelData, imageSize, rect.x + 1, rect.y + 1);
#endif
}

std::optional<TextureHandle> DynamicTexture::addImage(const uint8_t* pixelData, const Size& imageSize, int32_t uniqueId) {
    const auto& texHandle = reserveSize(imageSize, uniqueId);
    if (texHandle) {
        uploadImage(pixelData, *texHandle);
    }
    return texHandle;
}

void DynamicTexture::uploadDeferredImages() {
    for (const auto& pair : imagesToUpload) {
        const auto& rect = pair.first.getRectangle();
        texture->uploadSubRegion(pair.second.get(), Size(rect.w - 2, rect.h - 2), rect.x + 1, rect.y + 1);
    }
    imagesToUpload.clear();
}

void DynamicTexture::removeTexture(const TextureHandle& texHandle) {
    auto* bin = shelfPack.getBin(texHandle.getId());
    if (!bin) {
        return;
    }
    auto refcount = shelfPack.unref(*bin);
    if (refcount == 0) {
        numTextures--;
        imagesToUpload.erase(texHandle);
/*#if !defined(NDEBUG)
        Size size = Size(bin->w, bin->h);
        std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(size.area() * texture->numChannels());
        memset(data.get(), 0, size.area() * texture->numChannels());
        texture->uploadSubRegion(data.get(), size, bin->x, bin->y);
#endif*/
    }
}

} // namespace gfx
} // namespace mbgl
