#pragma once

#include <mln/shaders/layer_ubo.hpp>

#include <array>

namespace mln {
namespace shaders {

struct alignas(16) FillExtrusionDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 2> pixel_coord_upper;
    /*  72 */ std::array<float, 2> pixel_coord_lower;
    /*  80 */ float height_factor;
    /*  84 */ float tile_ratio;

    // Interpolations
    /*  88 */ float base_t;
    /*  92 */ float height_t;
    /*  96 */ float color_t;
    /* 100 */ float pattern_from_t;
    /* 104 */ float pattern_to_t;
    /* 108 */ float pad1;
    /* 112 */
};
static_assert(sizeof(FillExtrusionDrawableUBO) == 7 * 16);

struct alignas(16) FillExtrusionTilePropsUBO {
    /*  0 */ std::array<float, 4> pattern_from;
    /* 16 */ std::array<float, 4> pattern_to;
    /* 32 */ std::array<float, 2> texsize;
    // Heights of the first and last stop when fill-extrusion-color interpolates over ["elevation"] with
    // feature data, so the two per-vertex colors are blended by height; equal (0) otherwise.
    /* 40 */ float elevation_color_min;
    /* 44 */ float elevation_color_max;
    /* 48 */
};
static_assert(sizeof(FillExtrusionTilePropsUBO) == 3 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) FillExtrusionPropsUBO {
    /*  0 */ Color color;
    /* 16 */ std::array<float, 3> light_color;
    /* 28 */ float color_ramp_scale; // 1 / elevationColorRampMaxHeight if fill-extrusion-color uses elevation, else 0
    /* 32 */ std::array<float, 3> light_position;
    /* 44 */ float base;
    /* 48 */ float height;
    /* 52 */ float light_intensity;
    /* 56 */ float vertical_gradient;
    /* 60 */ float opacity;
    /* 64 */ float fade;
    /* 68 */ float from_scale;
    /* 72 */ float to_scale;
    /* 76 */ float color_ramp_zoom_t; // blend between the color ramp's two zoom rows
    /* 80 */
};
static_assert(sizeof(FillExtrusionPropsUBO) == 5 * 16);

/// `fill-extrusion-color` over `["elevation"]` without feature data is sampled into a lookup texture
/// covering [0, elevationColorRampMaxHeight] meters above the ground; heights above that clamp to the
/// last texel. It has two rows, at two zoom levels, see `ElevationColorRampZoom`.
constexpr float elevationColorRampMaxHeight = 512.0f;
constexpr uint32_t elevationColorRampSize = 2048;

} // namespace shaders
} // namespace mln
