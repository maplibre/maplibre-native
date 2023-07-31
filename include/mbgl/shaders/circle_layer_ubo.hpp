#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) CircleDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;        // composite model-view-projection matrix
    /* 64 */ std::array<float, 2> extrude_scale;
    /* 72 */ float pad;
    /* 80 */
};
static_assert(sizeof(CircleDrawableUBO) == 5 * 16);

struct alignas(16) CirclePaintParamsUBO {
    /*  0 */ float camera_to_center_distance;
    /*  4 */ float device_pixel_ratio;
    /*  8 */ float pad1, pad2;
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

struct alignas(16) CirclePermutationUBO {
    /*  0 */ Attribute color;
    /*  8 */ Attribute radius;
    /* 16 */ Attribute blur;
    /* 24 */ Attribute opacity;
    /* 32 */ Attribute stroke_color;
    /* 40 */ Attribute stroke_width;
    /* 48 */ Attribute stroke_opacity;
    /* 56 */ bool overdrawInspector;
    /* 57 */ std::array<uint8_t, 7> pad;
    /* 64 */
};
static_assert(sizeof(CirclePermutationUBO) == 4 * 16);

} // namespace shaders
} // namespace mbgl
