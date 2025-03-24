#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) CustomSymbolIconDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
    /*  64 */ std::array<float, 2> extrude_scale;
    /*  72 */ std::array<float, 2> anchor;
    /*  80 */ float angle_degrees;
    /*  84 */ uint32_t scale_with_map;
    /*  88 */ uint32_t pitch_with_map;
    /*  92 */ float camera_to_center_distance;
    /*  96 */ float aspect_ratio;
    /* 100 */ float pad1;
    /* 104 */ float pad2;
    /* 108 */ float pad3;
    /* 112 */
};
static_assert(sizeof(CustomSymbolIconDrawableUBO) == 7 * 16);

} // namespace shaders
} // namespace mbgl
