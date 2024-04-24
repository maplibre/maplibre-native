#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

#include <array>

namespace mbgl {
namespace shaders {

struct alignas(16) FillExtrusionDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ std::array<float, 2> texsize;
    /* 72 */ std::array<float, 2> pixel_coord_upper;
    /* 80 */ std::array<float, 2> pixel_coord_lower;
    /* 88 */ float height_factor;
    /* 92 */ float tile_ratio;
    /* 96 */
};
static_assert(sizeof(FillExtrusionDrawableUBO) == 6 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) FillExtrusionPropsUBO {
    /*  0 */ Color color;
    /* 16 */ std::array<float, 3> light_color;
    /* 28 */ float pad1;
    /* 32 */ std::array<float, 3> light_position;
    /* 44 */ float base;
    /* 48 */ float height;
    /* 52 */ float light_intensity;
    /* 56 */ float vertical_gradient;
    /* 60 */ float opacity;
    /* 64 */ float fade;
    /* 68 */ float from_scale;
    /* 72 */ float to_scale;
    /* 76 */ float pad2;
    /* 80 */
};
static_assert(sizeof(FillExtrusionPropsUBO) == 5 * 16);

/// Evaluated properties that depend on the tile
struct alignas(16) FillExtrusionTilePropsUBO {
    /*  0 */ std::array<float, 4> pattern_from;
    /* 16 */ std::array<float, 4> pattern_to;
    /* 32 */
};
static_assert(sizeof(FillExtrusionTilePropsUBO) == 2 * 16);

/// Attribute interpolations
struct alignas(16) FillExtrusionInterpolateUBO {
    /*  0 */ float base_t;
    /*  4 */ float height_t;
    /*  8 */ float color_t;
    /* 12 */ float pattern_from_t;
    /* 16 */ float pattern_to_t;
    /* 20 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(FillExtrusionInterpolateUBO) == 2 * 16);

enum {
    idFillExtrusionDrawableUBO = globalUBOCount,
    idFillExtrusionPropsUBO,
    idFillExtrusionTilePropsUBO,
    idFillExtrusionInterpolateUBO,
    fillExtrusionUBOCount
};

} // namespace shaders
} // namespace mbgl
