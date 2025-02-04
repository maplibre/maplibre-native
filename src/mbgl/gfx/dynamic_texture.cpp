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

std::optional<TextureHandle> DynamicTexture::addImage(const AlphaImage& image) {
    mapbox::Bin* bin = shelfPack.packOne(-1, image.size.width, image.size.height);
    if (!bin) {
        return std::nullopt;
    }
    textureAtlas->uploadSubRegion(image.data.get(), image.size, bin->x, bin->y);
    return TextureHandle(bin, this);
}

void DynamicTexture::removeTexture(TextureHandle& texHandle) {
    shelfPack.unref(*texHandle.getBin());
}

} // namespace gfx
} // namespace mbgl
