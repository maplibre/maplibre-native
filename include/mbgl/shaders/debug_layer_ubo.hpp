#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) DebugUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ Color color;
    /* 80 */ float overlay_scale;
    /* 84 */ float pad1;
    /* 88 */ float pad2;
    /* 92 */ float pad3;
    /* 96 */
};
static_assert(sizeof(DebugUBO) == 6 * 16);

} // namespace shaders
} // namespace mbgl
