#include <mbgl/gfx/dynamic_texture.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {
namespace gfx {

std::mutex mutex;

DynamicTexture::DynamicTexture(Context& context, Size size, TexturePixelType pixelType) {
    mapbox::ShelfPack::ShelfPackOptions options;
    options.autoResize = false;
    shelfPack = mapbox::ShelfPack(size.width, size.height, options);

    textureAtlas = context.createTexture2D();
    textureAtlas->setSize(size);
    textureAtlas->setFormat(pixelType, TextureChannelDataType::UnsignedByte);
    textureAtlas->setSamplerConfiguration(
        {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
    textureAtlas->create();
}

const Texture2DPtr& DynamicTexture::getTextureAtlas() {
    assert(textureAtlas);
    return textureAtlas;
}

std::optional<TextureHandle> DynamicTexture::addImage(const void* pixelData, const Size& imageSize, int32_t id) {
    mutex.lock();
    mapbox::Bin* bin = shelfPack.packOne(id, imageSize.width + 2, imageSize.height + 2);
    if (!bin) {
        mutex.unlock();
        return std::nullopt;
    }
    if (bin->refcount() == 1) {
        textureAtlas->uploadSubRegion(pixelData, imageSize, bin->x + 1, bin->y + 1);
    }
    mutex.unlock();
    return TextureHandle(bin);
}

void DynamicTexture::removeTexture(const TextureHandle& texHandle) {
    mutex.lock();
    auto refcount = shelfPack.unref(*texHandle.getBin());
#if !defined(NDEBUG)
    if (refcount == 0) {
        Size size = Size(texHandle.getBin()->w, texHandle.getBin()->h);
        std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(size.area() * textureAtlas->numChannels());
        memset(data.get(), 0, size.area() * textureAtlas->numChannels());
        textureAtlas->uploadSubRegion(data.get(), size, texHandle.getBin()->x, texHandle.getBin()->y);
    }
#endif
    mutex.unlock();
}

} // namespace gfx
} // namespace mbgl
