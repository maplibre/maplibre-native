#pragma once

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/renderer/layers/render_symbol_layer.hpp>

#include <memory>
#include <string>

namespace mbgl {

class TileAtlasTextures;
using TileAtlasTexturesPtr = std::shared_ptr<TileAtlasTextures>;

class PaintParameters;

namespace gfx {
class Drawable;

/**
 Tweaker that applies the latest values from a retained `TileAtlasTexturesPtr`
 */
class DrawableAtlasesTweaker : public gfx::DrawableTweaker {
public:
    DrawableAtlasesTweaker(TileAtlasTexturesPtr atlases_,
                           const std::optional<size_t> iconTextureId_,
                           const std::optional<size_t> glyphTextureId_,
                           bool isText_,
                           const bool sdfIcons_,
                           const style::AlignmentType rotationAlignment_,
                           const bool iconScaled_,
                           const bool textSizeIsZoomConstant_)
        : atlases(std::move(atlases_)),
          iconTextureId(iconTextureId_),
          glyphTextureId(glyphTextureId_),
          isText(isText_),
          sdfIcons(sdfIcons_),
          rotationAlignment(rotationAlignment_),
          iconScaled(iconScaled_),
          textSizeIsZoomConstant(textSizeIsZoomConstant_) {
        assert(iconTextureId_ != glyphTextureId_);
    }
    ~DrawableAtlasesTweaker() override = default;

    void init(Drawable&) override;

    void execute(Drawable&, PaintParameters&) override;

protected:
    void setupTextures(Drawable&, const bool);

    TileAtlasTexturesPtr atlases;
    std::optional<size_t> iconTextureId;
    std::optional<size_t> glyphTextureId;
    bool isText;

    const bool sdfIcons;
    const style::AlignmentType rotationAlignment;
    const bool iconScaled;
    const bool textSizeIsZoomConstant;
};

} // namespace gfx
} // namespace mbgl
