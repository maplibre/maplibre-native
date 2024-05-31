#include <mbgl/shaders/mtl/hillshade.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, true, sizeof(HillshadeDrawableUBO), idHillshadeDrawableUBO},
    UniformBlockInfo{false, true, sizeof(HillshadeEvaluatedPropsUBO), idHillshadeEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{hillshadeUBOCount + 0, gfx::AttributeDataType::Short2, idHillshadePosVertexAttribute},
    AttributeInfo{hillshadeUBOCount + 1, gfx::AttributeDataType::Short2, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idHillshadeImageTexture},
};

} // namespace shaders
} // namespace mbgl
