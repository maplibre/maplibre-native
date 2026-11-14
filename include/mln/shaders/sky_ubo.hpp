#pragma once

#include <mln/util/color.hpp>

#include <array>

namespace mln {
namespace shaders {

struct alignas(16) SkyPropsUBO {
    /*   0 */ std::array<float, 4 * 4> inv_projection;
    /*  64 */ Color sky_color;
    /*  80 */ Color horizon_color;
    // The globe's center in view space (xyz) and its radius (w), in pixels.
    /*  96 */ std::array<float, 4> globe;
    /* 112 */ std::array<float, 2> horizon;
    /* 120 */ std::array<float, 2> horizon_normal;
    /* 128 */ std::array<float, 2> viewport_size;
    /* 136 */ float sky_horizon_blend;
    /* 140 */ float sky_blend;
    /* 144 */ float camera_to_center_distance;
    /* 148 */ float pixel_ratio;
    /* 152 */ float atmosphere_blend;
    /* 156 */ float atmosphere_height;
    /* 160 */
};
static_assert(sizeof(SkyPropsUBO) == 10 * 16);

struct alignas(16) AtmospherePropsUBO {
    /*  0 */ std::array<float, 4 * 4> inv_projection;
    // The globe's center in view space (xyz) and its radius (w), in pixels.
    /* 64 */ std::array<float, 4> globe;
    // The fourth component stores the evaluated atmosphere blend.
    /* 80 */ std::array<float, 4> sun_position;
    /* 96 */
};
static_assert(sizeof(AtmospherePropsUBO) == 6 * 16);

} // namespace shaders
} // namespace mln
