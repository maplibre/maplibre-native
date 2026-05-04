#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>

namespace mbgl {
namespace gfx {

/**
 * @brief Drawable data for color-relief layer
 *
 * Stores per-tile DEM properties that are set during drawable creation
 * and read by the tweaker during rendering. This is necessary because
 * the DEM unpack vector and stride are only available from the bucket
 * during drawable creation.
 */
class ColorReliefDrawableData : public DrawableData {
public:
    explicit ColorReliefDrawableData(const shaders::ColorReliefTilePropsUBO& props)
        : tileProps(props) {}

    shaders::ColorReliefTilePropsUBO tileProps;
};

} // namespace gfx
} // namespace mbgl
