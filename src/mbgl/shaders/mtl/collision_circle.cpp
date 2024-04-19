#include <mbgl/shaders/mtl/collision_circle.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 1>
    ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{true, true, sizeof(CollisionUBO), idCollisionUBO},
};
const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::attributes =
    {
        AttributeInfo{collisionUBOCount + 0, gfx::AttributeDataType::Short2, idCollisionPosVertexAttribute},
        AttributeInfo{collisionUBOCount + 1, gfx::AttributeDataType::Short2, idCollisionAnchorPosVertexAttribute},
        AttributeInfo{collisionUBOCount + 2, gfx::AttributeDataType::Short2, idCollisionExtrudeVertexAttribute},
        AttributeInfo{collisionUBOCount + 3, gfx::AttributeDataType::UShort2, idCollisionPlacedVertexAttribute},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
