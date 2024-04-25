#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) RasterDrawableUBO {
    std::array<float, 4 * 4> matrix;
};
static_assert(sizeof(RasterDrawableUBO) == 64);

struct alignas(16) RasterEvaluatedPropsUBO {
    std::array<float, 4> spin_weights;
    std::array<float, 2> tl_parent;
    float scale_parent;
    float buffer_scale;
    float fade_t;
    float opacity;
    float brightness_low;
    float brightness_high;
    float saturation_factor;
    float contrast_factor;
    float pad1, pad2;
};
static_assert(sizeof(RasterEvaluatedPropsUBO) == 64);

enum {
    idRasterDrawableUBO = globalUBOCount,
    idRasterEvaluatedPropsUBO,
    rasterUBOCount
};

} // namespace shaders
} // namespace mbgl
