#include <mbgl/shaders/vulkan/debug.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 1> ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, true, sizeof(DebugUBO), idDebugUBO},
};
const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idDebugPosVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idDebugOverlayTexture},
};

} // namespace shaders
} // namespace mbgl
