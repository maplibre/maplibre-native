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
};
static_assert(sizeof(HillshadePrepareDrawableUBO) % 16 == 0);

} // namespace shaders
} // namespace mbgl
