#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HeatmapDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ float extrude_scale;

    // Interpolations
    /* 68 */ float weight_t;
    /* 72 */ float radius_t;
    /* 76 */ float pad1;
    /* 80 */
};
static_assert(sizeof(HeatmapDrawableUBO) == 5 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) HeatmapEvaluatedPropsUBO {
    /*  0 */ float weight;
    /*  4 */ float radius;
    /*  8 */ float intensity;
    /* 12 */ float padding;
    /* 16 */
};
static_assert(sizeof(HeatmapEvaluatedPropsUBO) == 16);

} // namespace shaders
} // namespace mbgl
