#include <mbgl/shaders/mtl/collision_circle.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::attributes =
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idCollisionPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::Short2, idCollisionAnchorPosVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::Short2, idCollisionExtrudeVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::UShort2, idCollisionPlacedVertexAttribute},
};
const std::array<UniformBlockInfo, 1>
    ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{4, true, true, sizeof(CollisionUBO), idCollisionUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
