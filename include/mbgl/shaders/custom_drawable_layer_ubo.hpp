#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

/// Custom Symbol Icon matrix
struct alignas(16) CustomSymbolIconDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */
};
static_assert(sizeof(CustomSymbolIconDrawableUBO) == 4 * 16);

/// Custom Symbol Icon Parameters
struct alignas(16) CustomSymbolIconParametersUBO {
    /*  0 */ std::array<float, 2> extrude_scale;
    /*  8 */ std::array<float, 2> anchor;
    /* 16 */ float angle_degrees;
    /* 20 */ uint32_t scale_with_map;
    /* 24 */ uint32_t pitch_with_map;
    /* 28 */ float camera_to_center_distance;
    /* 32 */ float aspect_ratio;
    /* 36 */ float pad0, pad1, pad2;
    /* 48 */
};
static_assert(sizeof(CustomSymbolIconParametersUBO) == 3 * 16);

enum {
    idCustomSymbolDrawableUBO,
    idCustomSymbolParametersUBO,
    customSymbolUBOCount
};

} // namespace shaders
} // namespace mbgl
