#pragma once

#include <mln/shaders/layer_ubo.hpp>

namespace mln {
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
    /* 32 */ float log2_world_width;
    /* 36 */ float pad1;
    /* 40 */ float pad2;
    /* 44 */ float pad3;
    /* 48 */
};
static_assert(sizeof(HillshadePrepareTilePropsUBO) == 3 * 16);

} // namespace shaders
} // namespace mln
