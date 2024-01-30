#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) RasterDrawableUBO {
    std::array<float, 4 * 4> matrix;
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
static_assert(sizeof(RasterDrawableUBO) == 128);
static_assert(sizeof(RasterDrawableUBO) % 16 == 0);

enum {
    idRasterDrawableUBO,
    rasterUBOCount
};

} // namespace shaders
} // namespace mbgl
