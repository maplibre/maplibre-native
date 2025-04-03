#include <mbgl/gfx/dynamic_texture.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {
namespace gfx {

std::mutex mutex;

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

std::optional<TextureHandle> DynamicTexture::addImage(const uint8_t* pixelData, const Size& imageSize, int32_t id) {
    mutex.lock();
    mapbox::Bin* bin = shelfPack.packOne(id, imageSize.width + 2, imageSize.height + 2);
    if (!bin) {
        mutex.unlock();
        return std::nullopt;
    }

    if (bin->refcount() == 1) {
#if MLN_DEFER_UPLOAD_ON_RENDER_THREAD
        auto size = imageSize.area() * texture->getPixelStride();
        auto imageData = std::make_unique<uint8_t[]>(size);
        std::copy(pixelData, pixelData + size, imageData.get());
        imagesToUpload.emplace(bin, std::move(imageData));
#else
        texture->uploadSubRegion(pixelData, imageSize, bin->x + 1, bin->y + 1);
#endif
    }
    mutex.unlock();
    return TextureHandle(bin);
}

void DynamicTexture::uploadDeferredImages() {
    mutex.lock();
    for (auto& pair : imagesToUpload) {
        const auto& bin = pair.first;
        texture->uploadSubRegion(pair.second.get(), Size(bin->w - 2, bin->h - 2), bin->x + 1, bin->y + 1);
    }
    imagesToUpload.clear();
    mutex.unlock();
}

void DynamicTexture::removeTexture(const TextureHandle& texHandle) {
    mutex.lock();
    const auto& bin = texHandle.getBin();
    auto refcount = shelfPack.unref(*bin);
    if (refcount == 0) {
        imagesToUpload.erase(bin);
#if !defined(NDEBUG)
        Size size = Size(bin->w, bin->h);
        std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(size.area() * texture->numChannels());
        memset(data.get(), 0, size.area() * texture->numChannels());
        texture->uploadSubRegion(data.get(), size, bin->x, bin->y);
#endif
    }
    mutex.unlock();
}

} // namespace gfx
} // namespace mbgl
