#pragma once

#include <mln/gfx/offscreen_texture.hpp>
#include <mln/gfx/types.hpp>

namespace mln {
namespace webgpu {

class Context;
class Texture2D;

class OffscreenTexture final : public gfx::OffscreenTexture {
public:
    OffscreenTexture(Context&, Size size, gfx::TextureChannelDataType type, bool depth, bool stencil);

    bool isRenderable() override;
    PremultipliedImage readStillImage() override;
    const gfx::Texture2DPtr& getTexture() override;
    gfx::Texture2DPtr takeTexture();
};

} // namespace webgpu
} // namespace mln
