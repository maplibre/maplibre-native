#include <mbgl/gfx/dynamic_texture.hpp>
#include <mbgl/gfx/context.hpp>

namespace mbgl {
namespace gfx {

DynamicTexture::DynamicTexture(Context& context, Size size) {
    mapbox::ShelfPack::ShelfPackOptions options;
    options.autoResize = false;
    shelfPack = mapbox::ShelfPack(size.width, size.height, options);

    textureAtlas = context.createTexture2D();
    textureAtlas->setSize(size);
    textureAtlas->setFormat(TexturePixelType::Alpha, TextureChannelDataType::UnsignedByte);
    textureAtlas->setSamplerConfiguration(
        {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
    textureAtlas->create();
}

const Texture2DPtr& DynamicTexture::getTextureAtlas() {
    assert(textureAtlas);
    return textureAtlas;
}

std::optional<TextureHandle> DynamicTexture::addImage(const AlphaImage& image, int32_t id) {
    mutex.lock();
    mapbox::Bin* bin = shelfPack.getBin(id);
    if (!bin) {
        bin = shelfPack.packOne(id, image.size.width, image.size.height);
        if (!bin) {
            mutex.unlock();
            return std::nullopt;
        }
        textureAtlas->uploadSubRegion(image.data.get(), image.size, bin->x, bin->y);
    }
    mutex.unlock();
    return TextureHandle(bin);
}

void DynamicTexture::removeTexture(const TextureHandle& texHandle) {
    mutex.lock();
#if !defined(NDEBUG)
    Size size = Size(texHandle.getBin()->w, texHandle.getBin()->h);
    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(size.area());
    memset(data.get(), 0, size.area());
    textureAtlas->uploadSubRegion(data.get(), size, texHandle.getBin()->x, texHandle.getBin()->y);
#endif
    shelfPack.unref(*texHandle.getBin());
    mutex.unlock();
}

} // namespace gfx
} // namespace mbgl
