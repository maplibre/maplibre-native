#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

#include <array>

namespace mbgl {
namespace shaders {

/// Evaluated properties that depend on the tile
struct alignas(16) FillExtrusionDrawableTilePropsUBO {
    /*  0 */ std::array<float, 4> pattern_from;
    /* 16 */ std::array<float, 4> pattern_to;
    /* 32 */
};
static_assert(sizeof(FillExtrusionDrawableTilePropsUBO) == 2 * 16);

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

struct alignas(16) FillExtrusionDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 4> scale;
    /*  80 */ std::array<float, 2> texsize;
    /*  88 */ std::array<float, 2> pixel_coord_upper;
    /*  96 */ std::array<float, 2> pixel_coord_lower;
    /* 104 */ float height_factor;
    /* 108 */ float pad;
    /* 112 */
};
static_assert(sizeof(FillExtrusionDrawableUBO) == 7 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) FillExtrusionDrawablePropsUBO {
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
    /* 68 */ float pad2, pad3, pad4;
    /* 80 */
};
static_assert(sizeof(FillExtrusionDrawablePropsUBO) == 5 * 16);

struct alignas(16) FillExtrusionPermutationUBO {
    /*  0 */ Attribute color;
    /*  8 */ Attribute base;
    /* 16 */ Attribute height;
    /* 24 */ bool overdrawInspector;
    /* 25 */ uint8_t pad1, pad2, pad3;
    /* 28 */ float pad4;
    /* 32 */
};
static_assert(sizeof(FillExtrusionPermutationUBO) == 2 * 16);

} // namespace shaders
} // namespace mbgl
