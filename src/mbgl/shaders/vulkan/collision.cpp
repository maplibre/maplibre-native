#include <mbgl/shaders/vulkan/collision.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 1> ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Vulkan>::uniforms =
    {
        UniformBlockInfo{true, true, sizeof(CollisionUBO), idCollisionUBO},
};
const std::array<AttributeInfo, 5> ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idCollisionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, idCollisionAnchorPosVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short2, idCollisionExtrudeVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::UShort2, idCollisionPlacedVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idCollisionShiftVertexAttribute},
};

const std::array<UniformBlockInfo, 1>
    ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{true, true, sizeof(CollisionUBO), idCollisionUBO},
};
const std::array<AttributeInfo, 4>
    ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Vulkan>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idCollisionPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::Short2, idCollisionAnchorPosVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::Short2, idCollisionExtrudeVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::UShort2, idCollisionPlacedVertexAttribute},
};

} // namespace shaders
} // namespace mbgl
