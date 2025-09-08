#pragma once

#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/style/property_expression.hpp>
#include <mbgl/util/bitmask_operations.hpp>
#include <mbgl/gfx/gpu_expression.hpp>

namespace mbgl {
namespace shaders {

//
// Line

struct alignas(16) LineDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ float ratio;

    // Interpolations
    /* 68 */ float color_t;
    /* 72 */ float blur_t;
    /* 76 */ float opacity_t;
    /* 80 */ float gapwidth_t;
    /* 84 */ float offset_t;
    /* 88 */ float width_t;
    /* 92 */ float pad1;
    /* 96 */
};
static_assert(sizeof(LineDrawableUBO) == 6 * 16);

//
// Line gradient

struct alignas(16) LineGradientDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ float ratio;

    // Interpolations
    /* 68 */ float blur_t;
    /* 72 */ float opacity_t;
    /* 76 */ float gapwidth_t;
    /* 80 */ float offset_t;
    /* 84 */ float width_t;
    /* 88 */ float pad1;
    /* 92 */ float pad2;
    /* 96 */
};
static_assert(sizeof(LineGradientDrawableUBO) == 6 * 16);

//
// Line pattern

struct alignas(16) LinePatternDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ float ratio;

    // Interpolations
    /* 68 */ float blur_t;
    /* 72 */ float opacity_t;
    /* 76 */ float gapwidth_t;
    /* 80 */ float offset_t;
    /* 84 */ float width_t;
    /* 88 */ float pattern_from_t;
    /* 92 */ float pattern_to_t;
    /* 96 */
};
static_assert(sizeof(LinePatternDrawableUBO) == 6 * 16);

struct alignas(16) LinePatternTilePropsUBO {
    /*  0 */ std::array<float, 4> pattern_from;
    /* 16 */ std::array<float, 4> pattern_to;
    /* 32 */ std::array<float, 4> scale;
    /* 48 */ std::array<float, 2> texsize;
    /* 56 */ float fade;
    /* 60 */ float pad1;
    /* 64 */
};
static_assert(sizeof(LinePatternTilePropsUBO) == 4 * 16);

//
// Line SDF

struct alignas(16) LineSDFDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 2> patternscale_a;
    /*  72 */ std::array<float, 2> patternscale_b;
    /*  80 */ float tex_y_a;
    /*  84 */ float tex_y_b;
    /*  88 */ float ratio;

    // Interpolations
    /*  92 */ float color_t;
    /*  96 */ float blur_t;
    /* 100 */ float opacity_t;
    /* 104 */ float gapwidth_t;
    /* 108 */ float offset_t;
    /* 112 */ float width_t;
    /* 116 */ float floorwidth_t;
    /* 120 */ float pad1;
    /* 124 */ float pad2;
    /* 128 */
};
static_assert(sizeof(LineSDFDrawableUBO) == 8 * 16);

struct alignas(16) LineSDFTilePropsUBO {
    /*  0 */ float sdfgamma;
    /*  4 */ float mix;
    /*  8 */ float pad1;
    /* 12 */ float pad2;
    /* 16 */
};
static_assert(sizeof(LineSDFTilePropsUBO) == 16);

/// Expression properties that do not depend on the tile
enum class LineExpressionMask : uint32_t {
    None = 0,
    Color = 1 << 0,
    Opacity = 1 << 1,
    Blur = 1 << 2,
    Width = 1 << 3,
    GapWidth = 1 << 4,
    FloorWidth = 1 << 5,
    Offset = 1 << 6,
};

struct alignas(16) LineExpressionUBO {
    gfx::GPUExpression color;
    gfx::GPUExpression blur;
    gfx::GPUExpression opacity;
    gfx::GPUExpression gapwidth;
    gfx::GPUExpression offset;
    gfx::GPUExpression width;
    gfx::GPUExpression floorWidth;
};
static_assert(sizeof(LineExpressionUBO) % 16 == 0);

/// Evaluated properties that do not depend on the tile
struct alignas(16) LineEvaluatedPropsUBO {
    /*  0 */ Color color;
    /* 16 */ float blur;
    /* 20 */ float opacity;
    /* 24 */ float gapwidth;
    /* 28 */ float offset;
    /* 32 */ float width;
    /* 36 */ float floorwidth;
    /* 40 */ LineExpressionMask expressionMask;
    /* 44 */ float pad1;
    /* 48 */
};
static_assert(sizeof(LineEvaluatedPropsUBO) == 3 * 16);

#if MLN_UBO_CONSOLIDATION

union LineDrawableUnionUBO {
    LineDrawableUBO lineDrawableUBO;
    LineGradientDrawableUBO lineGradientDrawableUBO;
    LinePatternDrawableUBO linePatternDrawableUBO;
    LineSDFDrawableUBO lineSDFDrawableUBO;
};

union LineTilePropsUnionUBO {
    LinePatternTilePropsUBO linePatternTilePropsUBO;
    LineSDFTilePropsUBO lineSDFTilePropsUBO;
};

#endif

} // namespace shaders
} // namespace mbgl
