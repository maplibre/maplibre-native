#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HillshadePrepareDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 4> unpack;
    std::array<float, 2> dimension;
    float zoom;
    float maxzoom;
    // overdrawInspector is used only in Metal, while in GL this is a 16 bytes empty padding.
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
    float pad4, pad5, pad6;
};
static_assert(sizeof(HillshadePrepareDrawableUBO) % 16 == 0);

} // namespace shaders
} // namespace mbgl
