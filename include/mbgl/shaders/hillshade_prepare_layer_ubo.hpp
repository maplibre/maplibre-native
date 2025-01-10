#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) HillshadePrepareDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */
};
static_assert(sizeof(HillshadePrepareDrawableUBO) == 4 * 16);

struct alignas(16) HillshadePrepareTilePropsUBO {
    /*  0 */ std::array<float, 4> unpack;
    /* 16 */ std::array<float, 2> dimension;
    /* 24 */ float zoom;
    /* 28 */ float maxzoom;
    /* 32 */
};
static_assert(sizeof(HillshadePrepareTilePropsUBO) == 2 * 16);

} // namespace shaders
} // namespace mbgl
