#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) CommonUBO {
    std::array<float, 4 * 4> matrix;
    Color color;
};
static_assert(sizeof(CommonUBO) % 16 == 0);

enum {
    idCommonUBO = globalUBOCount,
    commonUBOCount
};

} // namespace shaders
} // namespace mbgl
