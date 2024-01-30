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


enum {
    idFillDrawableUBO,
    idFillEvaluatedPropsUBO,
    idFillInterpolateUBO,
    idFillUBOCount
};

//
// Fill outline

struct alignas(16) FillOutlineDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix; // composite model-view-projection matrix
    /* 64 */ std::array<float, 2> world;
    /* 72 */ float pad1, pad2;
    /* 80 */
};
static_assert(sizeof(FillOutlineDrawableUBO) == 5 * 16);

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

enum {
    idFillOutlineDrawableUBO,
    idFillOutlineEvaluatedPropsUBO,
    idFillOutlineInterpolateUBO,
    idFillOutlineUBOCount
};

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

enum {
    idFillPatternDrawableUBO,
    idFillPatternTilePropsUBO,
    idFillPatternEvaluatedPropsUBO,
    idFillPatternInterpolateUBO,
    idFillPatternUBOCount
};

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

enum {
    idFillOutlinePatternDrawableUBO,
    idFillOutlinePatternTilePropsUBO,
    idFillOutlinePatternEvaluatedPropsUBO,
    idFillOutlinePatternInterpolateUBO,
    idFillOutlinePatternUBOCount
};

} // namespace shaders
} // namespace mbgl
