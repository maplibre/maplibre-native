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

struct alignas(16) FillPermutationUBO {
    /*   0 */ ColorAttribute color;
    /* 352 */ Attribute opacity;
    /* 360 */ int32_t /*bool*/ overdrawInspector;
    /* 364 */ float pad1;
    /* 368 */
};
static_assert(sizeof(FillPermutationUBO) == 23 * 16);

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

struct alignas(16) FillOutlinePermutationUBO {
    /*  0 */ Attribute outline_color;
    /*  8 */ Attribute opacity;
    /* 16 */ bool overdrawInspector;
    /* 17 */ bool pad1, pad2, pad3;
    /* 20 */ float pad4, pad5, pad6;
    /* 32 */
};
static_assert(sizeof(FillOutlinePermutationUBO) == 2 * 16);

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

struct alignas(16) FillPatternPermutationUBO {
    /*  0 */ Attribute pattern_from;
    /*  8 */ Attribute pattern_to;
    /* 16 */ Attribute opacity;
    /* 24 */ bool overdrawInspector;
    /* 25 */ bool pad1, pad2, pad3;
    /* 28 */ float pad4;
    /* 32 */
};
static_assert(sizeof(FillPatternPermutationUBO) == 2 * 16);

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

struct alignas(16) FillOutlinePatternPermutationUBO {
    /* 0  */ Attribute pattern_from;
    /* 8  */ Attribute pattern_to;
    /* 16 */ Attribute opacity;
    /* 24 */ bool overdrawInspector;
    /* 17 */ bool pad1, pad2, pad3;
    /* 20 */ float pad4;
    /* 32 */
};
static_assert(sizeof(FillOutlinePatternPermutationUBO) == 2 * 16);

} // namespace shaders
} // namespace mbgl
