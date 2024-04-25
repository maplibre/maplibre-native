#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

#include <memory>

namespace mbgl {
namespace shaders {

//
// Background

struct alignas(16) BackgroundDrawableUBO {
    std::array<float, 4 * 4> matrix;
};
static_assert(sizeof(BackgroundDrawableUBO) == 64);

struct alignas(16) BackgroundLayerUBO {
    /*  0 */ Color color;
    /* 16 */ float opacity;
    /* 24 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(BackgroundLayerUBO) == 32);

//
// Background pattern

struct alignas(16) BackgroundPatternDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> pixel_coord_upper;
    std::array<float, 2> pixel_coord_lower;
    float tile_units_to_pixels;
    float pad1, pad2, pad3;
};
static_assert(sizeof(BackgroundPatternDrawableUBO) == 96);

struct alignas(16) BackgroundPatternLayerUBO {
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
static_assert(sizeof(BackgroundPatternLayerUBO) == 64);

enum {
    idBackgroundDrawableUBO = globalUBOCount,
    idBackgroundLayerUBO,
    backgroundUBOCount
};

} // namespace shaders
} // namespace mbgl
