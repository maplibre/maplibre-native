#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Fill

struct alignas(16) FillEvaluatedPropsUBO {
    Color color;
    float opacity;
    float pad1, pad2, pad3;
};
static_assert(sizeof(FillEvaluatedPropsUBO) == 2 * 16);

struct alignas(16) FillInterpolateUBO {
    float color_t;
    float opacity_t;
    float pad1, pad2;
};
static_assert(sizeof(FillInterpolateUBO) % 16 == 0);

//
// Fill outline

struct alignas(16) FillOutlineDrawableUBO {
    /* 0 */ std::array<float, 2> world;
    /* 8 */ float pad1, pad2;
    /* 16 */
};
static_assert(sizeof(FillOutlineDrawableUBO) == 16);

struct alignas(16) FillOutlineEvaluatedPropsUBO {
    Color outline_color;
    float opacity;
    float pad1, pad2, pad3;
};
static_assert(sizeof(FillOutlineEvaluatedPropsUBO) == 2 * 16);

struct alignas(16) FillOutlineInterpolateUBO {
    float outline_color_t;
    float opacity_t;
    float pad1, pad2;
};
static_assert(sizeof(FillOutlineInterpolateUBO) == 1 * 16);

//
// Fill Pattern

struct alignas(16) FillPatternDrawableUBO {
    /*  0 */ std::array<float, 4> scale;
    /* 16 */ std::array<float, 2> pixel_coord_upper;
    /* 24 */ std::array<float, 2> pixel_coord_lower;
    /* 32 */ std::array<float, 2> texsize;
    /* 40 */ float pad1, pad2;
    /* 48 */
};
static_assert(sizeof(FillPatternDrawableUBO) == 3 * 16);

struct alignas(16) FillPatternEvaluatedPropsUBO {
    float opacity;
    float fade;
    float pad1, pad2;
};
static_assert(sizeof(FillPatternEvaluatedPropsUBO) == 1 * 16);

struct alignas(16) FillPatternTilePropsUBO {
    std::array<float, 4> pattern_from;
    std::array<float, 4> pattern_to;
};
static_assert(sizeof(FillPatternTilePropsUBO) == 2 * 16);

struct alignas(16) FillPatternInterpolateUBO {
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
    float pad1;
};
static_assert(sizeof(FillPatternInterpolateUBO) == 1 * 16);

//
// Fill pattern outline

struct alignas(16) FillOutlinePatternDrawableUBO {
    /*  0 */ std::array<float, 4> scale;
    /* 16 */ std::array<float, 2> world;
    /* 24 */ std::array<float, 2> pixel_coord_upper;
    /* 32 */ std::array<float, 2> pixel_coord_lower;
    /* 40 */ std::array<float, 2> texsize;
    /* 48 */
};
static_assert(sizeof(FillOutlinePatternDrawableUBO) == 3 * 16);

struct alignas(16) FillOutlinePatternEvaluatedPropsUBO {
    float opacity;
    float fade;
    float pad1, pad2;
};
static_assert(sizeof(FillOutlinePatternEvaluatedPropsUBO) == 16);

struct alignas(16) FillOutlinePatternTilePropsUBO {
    std::array<float, 4> pattern_from;
    std::array<float, 4> pattern_to;
};
static_assert(sizeof(FillOutlinePatternTilePropsUBO) == 2 * 16);

struct alignas(16) FillOutlinePatternInterpolateUBO {
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
    float pad;
};
static_assert(sizeof(FillOutlinePatternInterpolateUBO) == 1 * 16);

} // namespace shaders
} // namespace mbgl
