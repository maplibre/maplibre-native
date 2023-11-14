#include <mbgl/shaders/mtl/collision_circle.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::attributes =
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Short2, 1, "a_anchor_pos"},
        AttributeInfo{2, gfx::AttributeDataType::Short2, 1, "a_extrude"},
        AttributeInfo{3, gfx::AttributeDataType::UShort2, 1, "a_placed"},
};
const std::array<UniformBlockInfo, 1>
    ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{4, true, true, sizeof(CollisionUBO), "CollisionCircleUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
