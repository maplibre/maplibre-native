#include <mbgl/shaders/mtl/debug.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 1> ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, true, sizeof(DebugUBO), idDebugUBO},
};
const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{debugUBOCount + 0, gfx::AttributeDataType::Short2, idDebugPosVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idDebugOverlayTexture},
};

} // namespace shaders
} // namespace mbgl
