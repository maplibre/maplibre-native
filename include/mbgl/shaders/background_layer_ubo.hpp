#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

#include <memory>

namespace mbgl {
namespace shaders {

//
// Background

struct alignas(16) BackgroundDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */
};
static_assert(sizeof(BackgroundDrawableUBO) == 4 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) BackgroundPropsUBO {
    /*  0 */ Color color;
    /* 16 */ float opacity;
    /* 20 */ float pad1;
    /* 24 */ float pad2;
    /* 28 */ float pad3;
    /* 32 */
};
static_assert(sizeof(BackgroundPropsUBO) == 2 * 16);

//
// Background pattern

struct alignas(16) BackgroundPatternDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ std::array<float, 2> pixel_coord_upper;
    /* 72 */ std::array<float, 2> pixel_coord_lower;
    /* 80 */ float tile_units_to_pixels;
    /* 84 */ float pad1;
    /* 88 */ float pad2;
    /* 92 */ float pad3;
    /* 96 */
};
static_assert(sizeof(BackgroundPatternDrawableUBO) == 6 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) BackgroundPatternPropsUBO {
    /*  0 */ std::array<float, 2> pattern_tl_a;
    /*  8 */ std::array<float, 2> pattern_br_a;
    /* 16 */ std::array<float, 2> pattern_tl_b;
    /* 24 */ std::array<float, 2> pattern_br_b;
    /* 32 */ std::array<float, 2> pattern_size_a;
    /* 40 */ std::array<float, 2> pattern_size_b;
    /* 48 */ float scale_a;
    /* 52 */ float scale_b;
    /* 56 */ float mix;
    /* 60 */ float opacity;
    /* 64 */
};
static_assert(sizeof(BackgroundPatternPropsUBO) == 4 * 16);

#if MLN_UBO_CONSOLIDATION

union BackgroundDrawableUnionUBO {
    BackgroundDrawableUBO backgroundDrawableUBO;
    BackgroundPatternDrawableUBO backgroundPatternDrawableUBO;
};

#endif

} // namespace shaders
} // namespace mbgl
