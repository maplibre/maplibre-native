#include <mln/shaders/webgpu/collision.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/collision_layer_ubo.hpp>

namespace mln {
namespace shaders {

//
// Collision box

using CollisionBoxShaderSource = ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 5> CollisionBoxShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short2, idCollisionPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Short2, idCollisionAnchorPosVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short2, idCollisionExtrudeVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UShort2, idCollisionPlacedVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idCollisionShiftVertexAttribute},
};
const std::array<TextureInfo, 0> CollisionBoxShaderSource::textures = {};

//
// Collision circle

using CollisionCircleShaderSource = ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 4> CollisionCircleShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short2, idCollisionPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Short2, idCollisionAnchorPosVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short2, idCollisionExtrudeVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UShort2, idCollisionPlacedVertexAttribute},
};
const std::array<TextureInfo, 0> CollisionCircleShaderSource::textures = {};

} // namespace shaders
} // namespace mln
