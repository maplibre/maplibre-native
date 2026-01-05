#include <mbgl/mtl/dynamic_texture.hpp>
#include <mbgl/mtl/context.hpp>

namespace mbgl {
namespace mtl {

DynamicTexture::DynamicTexture(Context& context, Size size, gfx::TexturePixelType pixelType)
    : gfx::DynamicTexture(context, size, pixelType) {
    texture->create();
}

void DynamicTexture::uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) {
    std::scoped_lock lock(mutex);
    const auto& rect = texHandle.getRectangle();
    const auto imageSize = Size(rect.w, rect.h);

    texture->uploadSubRegion(pixelData, imageSize, rect.x, rect.y);

    gfx::DynamicTexture::uploadImage(pixelData, texHandle);
}

} // namespace mtl
} // namespace mbgl
