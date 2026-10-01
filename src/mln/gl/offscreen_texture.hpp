#pragma once

#include <mln/gfx/offscreen_texture.hpp>
#include <mln/gfx/types.hpp>

namespace mln {
namespace gl {

class Context;

class OffscreenTexture final : public gfx::OffscreenTexture {
public:
    OffscreenTexture(gl::Context&,
                     Size size,
                     gfx::TextureChannelDataType type = gfx::TextureChannelDataType::UnsignedByte);

    bool isRenderable() override;

    PremultipliedImage readStillImage() override;
    const gfx::Texture2DPtr& getTexture() override;
};

} // namespace gl
} // namespace mln
