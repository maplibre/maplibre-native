#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HeatmapDrawableUBO {
    std::array<float, 4 * 4> matrix;
    float extrude_scale;
    std::array<float, 3> padding;
};
static_assert(sizeof(HeatmapDrawableUBO) % 16 == 0);

struct alignas(16) HeatmapEvaluatedPropsUBO {
    float weight;
    float radius;
    float intensity;
    float padding;
};
static_assert(sizeof(HeatmapEvaluatedPropsUBO) % 16 == 0);

struct alignas(16) HeatmapInterpolateUBO {
    float weight_t;
    float radius_t;
    std::array<float, 2> padding;
};
static_assert(sizeof(HeatmapInterpolateUBO) % 16 == 0);

struct alignas(16) HeatmapPermutationUBO {
    /*  0 */ Attribute weight;
    /*  8 */ Attribute radius;
    /* 16 */ bool overdrawInspector;
    /* 17 */ uint8_t pad1, pad2, pad3;
    /* 20 */ float pad4, pad5, pad6;
    /* 32 */
};
static_assert(sizeof(HeatmapPermutationUBO) == 2 * 16);

} // namespace shaders
} // namespace mbgl
