#include <mbgl/shaders/webgpu/collision.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/collision_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Collision box

using CollisionBoxShaderSource = ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> CollisionBoxShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idCollisionAnchorPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idCollisionExtrudeVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idCollisionPlacedVertexAttribute},
};
const std::array<TextureInfo, 0> CollisionBoxShaderSource::textures = {};

//
// Collision circle

using CollisionCircleShaderSource = ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> CollisionCircleShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idCollisionAnchorPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idCollisionExtrudeVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idCollisionPlacedVertexAttribute},
};
const std::array<TextureInfo, 0> CollisionCircleShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl