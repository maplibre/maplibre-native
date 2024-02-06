#pragma once

#include <mbgl/gfx/drawable_tweaker.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/renderer/layers/render_symbol_layer.hpp>
#include <mbgl/util/string_indexer.hpp>

#include <memory>
#include <string>

namespace mbgl {

class TileAtlasTextures;
using TileAtlasTexturesPtr = std::shared_ptr<TileAtlasTextures>;

namespace gfx {

class Drawable;

/**
 Tweaker that applies the latest values from a retained `TileAtlasTexturesPtr`
 */
class DrawableAtlasesTweaker : public gfx::DrawableTweaker {
public:
    DrawableAtlasesTweaker(TileAtlasTexturesPtr atlases_,
                           const std::optional<StringIdentity> iconNameId_,
                           const std::optional<StringIdentity> glyphNameId_,
                           bool isText_,
                           const bool sdfIcons_,
                           const style::AlignmentType rotationAlignment_,
                           const bool iconScaled_,
                           const bool textSizeIsZoomConstant_)
        : atlases(std::move(atlases_)),
          iconNameId(iconNameId_),
          glyphNameId(glyphNameId_),
          isText(isText_),
          sdfIcons(sdfIcons_),
          rotationAlignment(rotationAlignment_),
          iconScaled(iconScaled_),
          textSizeIsZoomConstant(textSizeIsZoomConstant_) {
        assert(iconNameId_ != glyphNameId_);
    }
    ~DrawableAtlasesTweaker() override = default;

    void init(Drawable&) override;

    void execute(Drawable&, const PaintParameters&) override;

protected:
    void setupTextures(Drawable&, const bool);

    TileAtlasTexturesPtr atlases;
    std::optional<StringIdentity> iconNameId;
    std::optional<StringIdentity> glyphNameId;
    bool isText;

    const bool sdfIcons;
    const style::AlignmentType rotationAlignment;
    const bool iconScaled;
    const bool textSizeIsZoomConstant;
};

} // namespace gfx
} // namespace mbgl
