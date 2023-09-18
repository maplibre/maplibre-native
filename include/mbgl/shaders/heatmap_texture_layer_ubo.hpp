#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HeatmapTextureDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> world;
    float opacity;
    // overdrawInspector is used only in Metal, while in GL this is a 4 bytes empty padding.
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
};
static_assert(sizeof(HeatmapTextureDrawableUBO) % 16 == 0);

} // namespace shaders
} // namespace mbgl
