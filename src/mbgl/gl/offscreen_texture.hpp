#pragma once

#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/gfx/types.hpp>

namespace mbgl {
namespace gl {

class Context;

class OffscreenTexture final : public gfx::OffscreenTexture {
public:
    OffscreenTexture(gl::Context&,
                     Size size,
                     gfx::TextureChannelDataType type = gfx::TextureChannelDataType::UnsignedByte);

    bool isRenderable() override;

    PremultipliedImage readStillImage() override;

#if MLN_LEGACY_RENDERER
    gfx::Texture& getTexture() override;
#else
    const gfx::Texture2DPtr& getTexture() override;
#endif
};

} // namespace gl
} // namespace mbgl
