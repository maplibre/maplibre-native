#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HeatmapTexturePropsUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> world;
    float opacity;
    float pad1;
};
static_assert(sizeof(HeatmapTexturePropsUBO) % 16 == 0);

enum {
    idHeatmapTexturePropsUBO,
    heatmapTextureUBOCount
};

} // namespace shaders
} // namespace mbgl
