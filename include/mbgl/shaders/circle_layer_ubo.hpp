#pragma once

namespace mbgl {
namespace shaders {

struct alignas(16) CircleDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> extrude_scale;
    std::array<float, 2> padding;
};
static_assert(sizeof(CircleDrawableUBO) % 16 == 0);
static constexpr std::string_view CircleDrawableUBOName = "CircleDrawableUBO";

struct alignas(16) CirclePaintParamsUBO {
    float camera_to_center_distance;
    float device_pixel_ratio;
    std::array<float, 2> padding;
};
static_assert(sizeof(CirclePaintParamsUBO) % 16 == 0);
static constexpr std::string_view CirclePaintParamsUBOName = "CirclePaintParamsUBO";

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
static constexpr std::string_view CircleEvaluatedPropsUBOName = "CircleEvaluatedPropsUBO";

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
static constexpr auto CircleInterpolateUBOName = "CircleInterpolateUBO";

} // namespace shaders
} // namespace mbgl
