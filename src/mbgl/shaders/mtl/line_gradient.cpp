#include <mbgl/shaders/mtl/line_gradient.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 7> ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{7, true, false, sizeof(LineDynamicUBO), idLineGradientDynamicUBO},
    UniformBlockInfo{8, true, true, sizeof(LineGradientUBO), idLineGradientUBO},
    UniformBlockInfo{9, true, true, sizeof(LineGradientPropertiesUBO), idLineGradientPropertiesUBO},
    UniformBlockInfo{10, true, false, sizeof(LineGradientInterpolationUBO), idLineGradientInterpolationUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idLineImageTexture},
};

} // namespace shaders
} // namespace mbgl
