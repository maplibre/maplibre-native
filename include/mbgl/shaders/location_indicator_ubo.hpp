#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) LocationIndicatorDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ Color color;
    /* 80 */
};
static_assert(sizeof(LocationIndicatorDrawableUBO) == 5 * 16);

} // namespace shaders
} // namespace mbgl
