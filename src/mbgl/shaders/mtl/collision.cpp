#include <mbgl/shaders/mtl/collision.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Collision box

using CollisionBoxShaderSource = ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 5> CollisionBoxShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, collisionUBOCount + 0, idCollisionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, collisionUBOCount + 0, idCollisionAnchorPosVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short2, collisionUBOCount + 0, idCollisionExtrudeVertexAttribute},
    
    // Dynamic
    AttributeInfo{3, gfx::AttributeDataType::UShort2, collisionUBOCount + 1, idCollisionPlacedVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, collisionUBOCount + 1, idCollisionShiftVertexAttribute},
};
const std::array<TextureInfo, 0> CollisionBoxShaderSource::textures = {};

//
// Collision circle

using CollisionCircleShaderSource = ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 4> CollisionCircleShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, collisionUBOCount + 0, idCollisionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, collisionUBOCount + 0, idCollisionAnchorPosVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short2, collisionUBOCount + 0, idCollisionExtrudeVertexAttribute},
    
    // Dynamic
    AttributeInfo{3, gfx::AttributeDataType::UShort2, collisionUBOCount + 1, idCollisionPlacedVertexAttribute},
};
const std::array<TextureInfo, 0> CollisionCircleShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
