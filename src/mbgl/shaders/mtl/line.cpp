#include <mbgl/shaders/mtl/line.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, false, sizeof(idLineDrawableUBO), idLineDrawableUBO},
    UniformBlockInfo{true, false, sizeof(LineInterpolationUBO), idLineInterpolationUBO},
    UniformBlockInfo{true, true, sizeof(LineEvaluatedPropsUBO), idLineEvaluatedPropsUBO},
    UniformBlockInfo{true, true, sizeof(LineExpressionUBO), idLineExpressionUBO},
};
const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{lineUBOCount + 2, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{lineUBOCount + 3, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{lineUBOCount + 4, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{lineUBOCount + 5, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{lineUBOCount + 6, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{lineUBOCount + 7, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, false, sizeof(LineGradientDrawableUBO), idLineDrawableUBO},
    UniformBlockInfo{true, false, sizeof(LineGradientInterpolationUBO), idLineInterpolationUBO},
    UniformBlockInfo{true, true, sizeof(LineEvaluatedPropsUBO), idLineEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 7> ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{lineUBOCount + 2, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{lineUBOCount + 3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{lineUBOCount + 4, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{lineUBOCount + 5, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{lineUBOCount + 6, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idLineImageTexture},
};

const std::array<UniformBlockInfo, 6> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, true, sizeof(LinePatternDrawableUBO), idLineDrawableUBO},
    UniformBlockInfo{true, false, sizeof(LinePatternInterpolationUBO), idLineInterpolationUBO},
    UniformBlockInfo{true, true, sizeof(LinePatternTilePropertiesUBO), idLineTilePropertiesUBO},
    UniformBlockInfo{true, true, sizeof(LineEvaluatedPropsUBO), idLineEvaluatedPropsUBO},
    UniformBlockInfo{true, true, sizeof(LineExpressionUBO), idLineExpressionUBO},
};
const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{lineUBOCount + 2, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{lineUBOCount + 3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{lineUBOCount + 4, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{lineUBOCount + 5, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{lineUBOCount + 6, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
    AttributeInfo{lineUBOCount + 7, gfx::AttributeDataType::UShort4, idLinePatternFromVertexAttribute},
    AttributeInfo{lineUBOCount + 8, gfx::AttributeDataType::UShort4, idLinePatternToVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idLineImageTexture},
};

const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, true, sizeof(LineSDFDrawableUBO), idLineDrawableUBO},
    UniformBlockInfo{true, false, sizeof(LineSDFInterpolationUBO), idLineInterpolationUBO},
    UniformBlockInfo{true, true, sizeof(LineEvaluatedPropsUBO), idLineEvaluatedPropsUBO},
    UniformBlockInfo{true, true, sizeof(LineExpressionUBO), idLineExpressionUBO},
};
const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{lineUBOCount + 2, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{lineUBOCount + 3, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{lineUBOCount + 4, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{lineUBOCount + 5, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{lineUBOCount + 6, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{lineUBOCount + 7, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
    AttributeInfo{lineUBOCount + 8, gfx::AttributeDataType::Float2, idLineFloorWidthVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idLineImageTexture},
};

} // namespace shaders
} // namespace mbgl
