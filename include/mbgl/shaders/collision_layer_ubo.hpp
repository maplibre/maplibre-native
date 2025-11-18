#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) CollisionDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */
};
static_assert(sizeof(CollisionDrawableUBO) == 4 * 16);

struct alignas(16) CollisionTilePropsUBO {
    /*  0 */ std::array<float, 2> extrude_scale;
    /*  8 */ float overscale_factor;
    /* 12 */ float pad1;
    /* 16 */
};
static_assert(sizeof(CollisionTilePropsUBO) == 16);

} // namespace shaders
} // namespace mbgl
