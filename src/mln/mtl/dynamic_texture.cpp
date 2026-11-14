#include <mln/mtl/dynamic_texture.hpp>
#include <mln/mtl/texture2d.hpp>
#include <mln/mtl/context.hpp>

namespace mln {
namespace mtl {

DynamicTexture::DynamicTexture(Context& context, Size size, gfx::TexturePixelType pixelType)
    : gfx::DynamicTexture(context, size, pixelType) {}

void DynamicTexture::uploadImage(const uint8_t* pixelData, gfx::TextureHandle& texHandle) {
    std::scoped_lock lock(mutex);
    const auto& rect = texHandle.getRectangle();
    const auto imageSize = Size(rect.w, rect.h);

    // Worker threads upload into a texture the render thread is drawing with. The mutex only orders the uploads,
    // so leave the sampler state to the render thread, which updates it in `Texture2D::bind`.
    static_cast<Texture2D&>(*texture).createTexture();
    texture->uploadSubRegion(pixelData, imageSize, rect.x, rect.y);

    gfx::DynamicTexture::uploadImage(pixelData, texHandle);
}

} // namespace mtl
} // namespace mln
