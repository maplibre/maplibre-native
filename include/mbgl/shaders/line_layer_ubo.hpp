#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Line

struct alignas(16) LineDynamicUBO {
    /* 0 */ std::array<float, 2> units_to_pixels;
    /* 8 */ float pad1, pad2;
    /* 16 */
};
static_assert(sizeof(LineDynamicUBO) == 16);

struct alignas(16) LineDrawableUBO {
    std::array<float, 4 * 4> matrix;
    float ratio;
    float pad1, pad2, pad3;
};
static_assert(sizeof(LineDrawableUBO) % 16 == 0);

struct alignas(16) LineInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1, pad2;
};
static_assert(sizeof(LineInterpolationUBO) % 16 == 0);

//
// Line gradient

using LineGradientDrawableUBO = LineDrawableUBO;

struct alignas(16) LineGradientInterpolationUBO {
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1, pad2, pad3;
};
static_assert(sizeof(LineGradientInterpolationUBO) % 16 == 0);

//
// Line pattern

struct alignas(16) LinePatternDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 4> scale;
    std::array<float, 2> texsize;
    float ratio;
    float fade;
};
static_assert(sizeof(LinePatternDrawableUBO) % 16 == 0);

struct alignas(16) LinePatternInterpolationUBO {
    float blur_t;
    float opacity_t;
    float offset_t;
    float gapwidth_t;
    float width_t;
    float pattern_from_t;
    float pattern_to_t;
    float pad1;
};
static_assert(sizeof(LinePatternInterpolationUBO) % 16 == 0);

struct alignas(16) LinePatternTilePropertiesUBO {
    std::array<float, 4> pattern_from;
    std::array<float, 4> pattern_to;
};
static_assert(sizeof(LinePatternTilePropertiesUBO) % 16 == 0);

//
// Line SDF

struct alignas(16) LineSDFDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> patternscale_a;
    std::array<float, 2> patternscale_b;
    float ratio;
    float tex_y_a;
    float tex_y_b;
    float sdfgamma;
    float mix;
    float pad1, pad2, pad3;
};
static_assert(sizeof(LineSDFDrawableUBO) % 16 == 0);

struct alignas(16) LineSDFInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float floorwidth_t;
    float pad1;
};
static_assert(sizeof(LineSDFInterpolationUBO) % 16 == 0);

//
// Line evaluated properties

struct alignas(16) LineEvaluatedPropsUBO {
    Color color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    float pad1, pad2;
};
static_assert(sizeof(LineEvaluatedPropsUBO) % 16 == 0);

enum {
    idLineDynamicUBO,
    idLineDrawableUBO,
    idLineInterpolationUBO,
    idLineTilePropertiesUBO,
    idLineEvaluatedPropsUBO,
    lineUBOCount
};

} // namespace shaders
} // namespace mbgl
