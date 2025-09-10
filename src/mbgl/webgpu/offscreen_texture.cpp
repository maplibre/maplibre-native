#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/webgpu/renderbuffer.hpp>

namespace mbgl {
namespace webgpu {

OffscreenTexture::OffscreenTexture(Context& context_, Size size_)
    : gfx::OffscreenTexture(size_, std::make_unique<RenderableResource>()),
      context(context_) {
    texture = context.createTexture2D();
    texture->setSize(size_);
    texture->create();
}

OffscreenTexture::~OffscreenTexture() = default;

bool OffscreenTexture::isRenderable() {
    return true;
}

PremultipliedImage OffscreenTexture::readStillImage() {
    // TODO: Read texture data from WebGPU
    return PremultipliedImage(getSize());
}

const gfx::Texture2DPtr& OffscreenTexture::getTexture() {
    return texture;
}

} // namespace webgpu
} // namespace mbgl