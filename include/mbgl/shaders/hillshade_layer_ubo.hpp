#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HillshadeDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> latrange;
    std::array<float, 2> light;
    // overdrawInspector is used only in Metal, while in GL this is a 16 bytes empty padding.
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
    float pad4, pad5, pad6;
};
static_assert(sizeof(HillshadeDrawableUBO) % 16 == 0);

struct alignas(16) HillshadeEvaluatedPropsUBO {
    Color highlight;
    Color shadow;
    Color accent;
};
static_assert(sizeof(HillshadeEvaluatedPropsUBO) % 16 == 0);

} // namespace shaders
} // namespace mbgl
