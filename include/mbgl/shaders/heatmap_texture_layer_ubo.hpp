#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HeatmapTextureDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> world;
    float opacity;
    float pad1;
};
static_assert(sizeof(HeatmapTextureDrawableUBO) % 16 == 0);

enum {
    idHeatmapTextureDrawableUBO,
    heatmapTextureUBOCount
};

} // namespace shaders
} // namespace mbgl
