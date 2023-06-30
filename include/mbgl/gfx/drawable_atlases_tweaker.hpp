#pragma once

#include <mbgl/gfx/drawable_tweaker.hpp>

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
                           std::string iconName_,
                           std::string glyphName_,
                           bool isText_) :
        atlases(std::move(atlases_)),
        iconName(std::move(iconName_)),
        glyphName(std::move(glyphName_)),
        isText(isText_) { }
    ~DrawableAtlasesTweaker() override = default;
    
    void execute(Drawable&, const PaintParameters&) override;
    
protected:
    TileAtlasTexturesPtr atlases;
    std::string iconName;
    std::string glyphName;
    bool isText;
};

} // namespace gfx
} // namespace mbgl
