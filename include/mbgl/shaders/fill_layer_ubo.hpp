#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Fill

struct alignas(16) FillDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix; // composite model-view-projection matrix
};
static_assert(sizeof(FillDrawableUBO) == 4 * 16);

struct alignas(16) FillInterpolateUBO {
    float color_t;
    float opacity_t;
    float pad1, pad2;
};
static_assert(sizeof(FillInterpolateUBO) % 16 == 0);

//
// Fill outline

struct alignas(16) FillOutlineDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix; // composite model-view-projection matrix
    /* 64 */ std::array<float, 2> world;
    /* 72 */ float pad1, pad2;
    /* 80 */
};
static_assert(sizeof(FillOutlineDrawableUBO) == 5 * 16);

struct alignas(16) FillOutlineInterpolateUBO {
    float outline_color_t;
    float opacity_t;
    float pad1, pad2;
};
static_assert(sizeof(FillOutlineInterpolateUBO) == 1 * 16);

//
// Fill Pattern

struct alignas(16) FillPatternDrawableUBO {
    /*  0  */ std::array<float, 4 * 4> matrix; // composite model-view-projection matrix
    /*  64 */ std::array<float, 4> scale;
    /*  80 */ std::array<float, 2> pixel_coord_upper;
    /*  88 */ std::array<float, 2> pixel_coord_lower;
    /*  96 */ std::array<float, 2> texsize;
    /* 104 */ float pad1, pad2;
    /* 112 */
};
static_assert(sizeof(FillPatternDrawableUBO) == 7 * 16);

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
    /*  0  */ std::array<float, 4 * 4> matrix; // composite model-view-projection matrix
    /*  64 */ std::array<float, 4> scale;
    /*  80 */ std::array<float, 2> world;
    /*  88 */ std::array<float, 2> pixel_coord_upper;
    /*  96 */ std::array<float, 2> pixel_coord_lower;
    /* 104 */ std::array<float, 2> texsize;
};
static_assert(sizeof(FillOutlinePatternDrawableUBO) == 7 * 16);

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

//
// Fill outline triangulated

struct alignas(16) FillOutlineTriangulatedDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> units_to_pixels;
    float ratio;
    float pad;
};
static_assert(sizeof(FillOutlineTriangulatedDrawableUBO) % 16 == 0);

//
// Fill evaluated properties

struct alignas(16) FillEvaluatedPropsUBO {
    Color color;
    Color outline_color;
    float opacity;
    float fade;
    float width;
    float pad1;
};
static_assert(sizeof(FillEvaluatedPropsUBO) == 3 * 16);

enum {
    idFillDrawableUBO,
    idFillTilePropsUBO,
    idFillInterpolateUBO,
    idFillEvaluatedPropsUBO,
    fillUBOCount
};

} // namespace shaders
} // namespace mbgl
