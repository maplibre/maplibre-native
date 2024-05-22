#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) CollisionUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> extrude_scale;
    float overscale_factor;
    float pad;
};
static_assert(sizeof(CollisionUBO) % 16 == 0);
static_assert(sizeof(CollisionUBO) == 80);

enum {
    idCollisionUBO = globalUBOCount,
    collisionUBOCount
};

} // namespace shaders
} // namespace mbgl
