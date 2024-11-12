#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) DebugUBO {
    std::array<float, 4 * 4> matrix;
    Color color;
    float overlay_scale;
    float pad1, pad2, pad3;
};
static_assert(sizeof(DebugUBO) % 16 == 0);

} // namespace shaders
} // namespace mbgl
