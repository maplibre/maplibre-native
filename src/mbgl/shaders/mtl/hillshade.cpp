#include <mbgl/shaders/mtl/hillshade.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idHillshadePosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, idHillshadeTexturePosVertexAttribute},
};
const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{2, true, true, sizeof(HillshadeDrawableUBO), idHillshadeDrawableUBO},
    UniformBlockInfo{3, false, true, sizeof(HillshadeEvaluatedPropsUBO), idHillshadeEvaluatedPropsUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idHillshadeImageTexture},
};

} // namespace shaders
} // namespace mbgl
