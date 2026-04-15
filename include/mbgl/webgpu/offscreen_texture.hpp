#pragma once

#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/gfx/types.hpp>

namespace mbgl {
namespace webgpu {

class Context;
class Texture2D;

class OffscreenTexture final : public gfx::OffscreenTexture {
public:
    OffscreenTexture(Context&, Size size, gfx::TextureChannelDataType type, bool depth, bool stencil);

    bool isRenderable() override;
    PremultipliedImage readStillImage() override;
    const gfx::Texture2DPtr& getTexture() override;
};

} // namespace webgpu
} // namespace mbgl
