#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) CircleDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix; // composite model-view-projection matrix
    /* 64 */ std::array<float, 2> extrude_scale;
    /* 72 */ float pad;
    /* 80 */
};
static_assert(sizeof(CircleDrawableUBO) == 5 * 16);

struct alignas(16) CirclePaintParamsUBO {
    /*  0 */ float camera_to_center_distance;
    /*  4 */ float pad1, pad2, pad3;
    /* 16 */
};
static_assert(sizeof(CirclePaintParamsUBO) == 1 * 16);

struct alignas(16) CircleEvaluatedPropsUBO {
    Color color;
    Color stroke_color;
    float radius;
    float blur;
    float opacity;
    float stroke_width;
    float stroke_opacity;
    int scale_with_map;
    int pitch_with_map;
    float padding;
};
static_assert(sizeof(CircleEvaluatedPropsUBO) % 16 == 0);

struct alignas(16) CircleInterpolateUBO {
    float color_t;
    float radius_t;
    float blur_t;
    float opacity_t;
    float stroke_color_t;
    float stroke_width_t;
    float stroke_opacity_t;
    float padding;
};
static_assert(sizeof(CircleInterpolateUBO) % 16 == 0);

enum {
    idCircleDrawableUBO = globalUBOCount,
    idCircleEvaluatedPropsUBO,
    idCircleInterpolateUBO,
    circleUBOCount
};

} // namespace shaders
} // namespace mbgl
