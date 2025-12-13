#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>

#include <memory>

namespace mbgl {

namespace gfx {

class ColorReliefDrawableData : public DrawableData {
public:
    ColorReliefDrawableData(const shaders::ColorReliefTilePropsUBO& tileProps_)
        : tileProps(tileProps_) {}

    shaders::ColorReliefTilePropsUBO tileProps;
};

using UniqueColorReliefDrawableData = std::unique_ptr<ColorReliefDrawableData>;

} // namespace gfx
} // namespace mbgl
