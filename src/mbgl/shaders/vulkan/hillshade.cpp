#include <mbgl/shaders/vulkan/hillshade.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 1>
    ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{true, true, sizeof(HillshadePrepareDrawableUBO), idHillshadePrepareDrawableUBO},
};
const std::array<AttributeInfo, 2>
    ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Vulkan>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idHillshadePosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::Short2, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idHillshadeImageTexture},
};

const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, true, sizeof(HillshadeDrawableUBO), idHillshadeDrawableUBO},
    UniformBlockInfo{false, true, sizeof(HillshadeEvaluatedPropsUBO), idHillshadeEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idHillshadePosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idHillshadeImageTexture},
};

} // namespace shaders
} // namespace mbgl
