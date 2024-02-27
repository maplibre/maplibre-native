#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) CollisionUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> extrude_scale;
    float camera_to_center_distance;
    float overscale_factor;
};
static_assert(sizeof(CollisionUBO) % 16 == 0);
static_assert(sizeof(CollisionUBO) == 80);

enum {
    idCollisionUBO,
    collisionUBOCount
};

} // namespace shaders
} // namespace mbgl
