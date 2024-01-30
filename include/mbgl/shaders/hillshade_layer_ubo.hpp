#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HillshadeDrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> latrange;
    std::array<float, 2> light;
};
static_assert(sizeof(HillshadeDrawableUBO) == 16 * 5);

struct alignas(16) HillshadeEvaluatedPropsUBO {
    Color highlight;
    Color shadow;
    Color accent;
};
static_assert(sizeof(HillshadeEvaluatedPropsUBO) % 16 == 0);

enum {
    idHillshadeDrawableUBO,
    idHillshadeEvaluatedPropsUBO,
    idHillshadeUBOCount
};

} // namespace shaders
} // namespace mbgl
