#include <mbgl/shaders/mtl/debug.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos"},
};
const std::array<UniformBlockInfo, 1> ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{1, true, true, sizeof(DebugUBO), "DebugUBO"},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_overlay"},
};

} // namespace shaders
} // namespace mbgl
