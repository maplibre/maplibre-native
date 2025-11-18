#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HillshadeDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */
};
static_assert(sizeof(HillshadeDrawableUBO) == 4 * 16);

struct alignas(16) HillshadeTilePropsUBO {
    /*  0 */ std::array<float, 2> latrange;
    /*  8 */ std::array<float, 2> light;
    /* 16 */
};
static_assert(sizeof(HillshadeTilePropsUBO) == 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) HillshadeEvaluatedPropsUBO {
    /*  0 */ Color highlight;
    /* 16 */ Color shadow;
    /* 32 */ Color accent;
    /* 48 */
};
static_assert(sizeof(HillshadeEvaluatedPropsUBO) == 3 * 16);

} // namespace shaders
} // namespace mbgl
