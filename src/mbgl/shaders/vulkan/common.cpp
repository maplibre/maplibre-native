#include <mbgl/shaders/vulkan/common.hpp>
#include <mbgl/shaders/common_ubo.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 1> ShaderSource<BuiltIn::CommonShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, true, sizeof(CommonUBO), idCommonUBO},
};
const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::CommonShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idCommonPosVertexAttribute},
};

const std::array<UniformBlockInfo, 1>
    ShaderSource<BuiltIn::CommonTexturedShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{true, true, sizeof(CommonUBO), idCommonUBO},
};
const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::CommonTexturedShader, gfx::Backend::Type::Vulkan>::attributes =
    {
        AttributeInfo{0, gfx::AttributeDataType::Float2, idCommonPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::Float2, idCommonTexVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::CommonTexturedShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idCommonTexture},
};

} // namespace shaders
} // namespace mbgl
