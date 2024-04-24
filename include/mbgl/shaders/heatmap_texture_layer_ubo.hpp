#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HeatmapTexturePropsUBO {
    std::array<float, 4 * 4> matrix;
    float opacity;
    float pad1, pad2, pad3;
};
static_assert(sizeof(HeatmapTexturePropsUBO) % 16 == 0);

enum {
    idHeatmapTexturePropsUBO = globalUBOCount,
    heatmapTextureUBOCount
};

} // namespace shaders
} // namespace mbgl
