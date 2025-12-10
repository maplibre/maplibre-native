#pragma once
#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) ColorReliefDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */
};
static_assert(sizeof(ColorReliefDrawableUBO) == 64);

struct alignas(16) ColorReliefTilePropsUBO {
    /*  0 */ std::array<float, 4> unpack;    // DEM unpack vector
    /* 16 */ std::array<float, 2> dimension; // Texture dimensions
    /* 24 */ int32_t color_ramp_size;        // Number of color stops
    /* 28 */ float pad_tile0;                // Padding for alignment
    /* 32 */
};
static_assert(sizeof(ColorReliefTilePropsUBO) == 32);

struct alignas(16) ColorReliefEvaluatedPropsUBO {
    /*  0 */ float opacity;
    /*  4 */ float pad_eval0;
    /*  8 */ float pad_eval1;
    /* 12 */ float pad_eval2;
    /* 16 */
};
static_assert(sizeof(ColorReliefEvaluatedPropsUBO) == 16);

} // namespace shaders
} // namespace mbgl
