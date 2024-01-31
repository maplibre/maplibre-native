#include <mbgl/shaders/mtl/collision_box.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 5> ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Short2, "a_anchor_pos"},
    AttributeInfo{2, gfx::AttributeDataType::Short2, "a_extrude"},
    AttributeInfo{3, gfx::AttributeDataType::UShort2, "a_placed"},
    AttributeInfo{4, gfx::AttributeDataType::Float2, "a_shift"},
};
const std::array<UniformBlockInfo, 1> ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{5, true, true, sizeof(CollisionUBO), idCollisionUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
