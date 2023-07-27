#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <memory>

namespace mbgl {
namespace shaders {

struct alignas(16) BackgroundDrawableUBO {
    std::array<float, 4 * 4> matrix;
};
static_assert(sizeof(BackgroundDrawableUBO) % 16 == 0);
static constexpr auto BackgroundDrawableUBOName = "BackgroundDrawableUBO";


struct alignas(16) BackgroundLayerUBO {
    /*  0 */ Color color;
    /* 16 */ float opacity;
    /* 20 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(BackgroundLayerUBO) == 32);
static constexpr auto BackgroundLayerUBOName = "BackgroundLayerUBO";


struct alignas(16) BackgroundPatternLayerUBO {
    /*  0 */ std::array<float, 2> pattern_tl_a;
    /*  8 */ std::array<float, 2> pattern_br_a;
    /* 16 */ std::array<float, 2> pattern_tl_b;
    /* 24 */ std::array<float, 2> pattern_br_b;
    /* 32 */ std::array<float, 2> texsize;
    /* 40 */ std::array<float, 2> pattern_size_a;
    /* 48 */ std::array<float, 2> pattern_size_b;
    /* 56 */ std::array<float, 2> pixel_coord_upper;
    /* 64 */ std::array<float, 2> pixel_coord_lower;
    /* 72 */ float tile_units_to_pixels;
    /* 76 */ float scale_a;
    /* 80 */ float scale_b;
    /* 84 */ float mix;
    /* 88 */ float opacity;
    /* 92 */ float pad;
    /* 96 */
};
static_assert(sizeof(BackgroundPatternLayerUBO) == 96);
static constexpr auto BackgroundPatternLayerUBOName = "BackgroundPatternLayerUBO";

} // namespace shaders
} // namespace mbgl
