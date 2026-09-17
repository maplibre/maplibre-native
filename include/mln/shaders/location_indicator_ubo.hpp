#pragma once

#include <mln/shaders/layer_ubo.hpp>

namespace mln {
namespace shaders {

struct alignas(16) LocationIndicatorDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ Color color;
    // Half-angle in radians, sector enabled, padding. Zero disables the sector mask.
    /* 80 */ std::array<float, 4> sector{};
    /* 96 */
};
static_assert(sizeof(LocationIndicatorDrawableUBO) == 6 * 16);

} // namespace shaders
} // namespace mln
