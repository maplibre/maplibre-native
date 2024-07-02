#include <mbgl/shaders/vulkan/line.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, false, sizeof(LineDrawableUBO), idLineDrawableUBO},
    UniformBlockInfo{true, false, sizeof(LineInterpolationUBO), idLineInterpolationUBO},
    UniformBlockInfo{true, true, sizeof(LineEvaluatedPropsUBO), idLineEvaluatedPropsUBO},
};

const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Vulkan>::uniforms =
    {
        UniformBlockInfo{true, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, false, sizeof(LineDrawableUBO), idLineDrawableUBO},
        UniformBlockInfo{true, false, sizeof(LineInterpolationUBO), idLineInterpolationUBO},
        UniformBlockInfo{true, true, sizeof(LineEvaluatedPropsUBO), idLineEvaluatedPropsUBO},
};

const std::array<AttributeInfo, 7> ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};

const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idLineImageTexture},
};

const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, true, sizeof(LinePatternDrawableUBO), idLineDrawableUBO},
    UniformBlockInfo{true, false, sizeof(LinePatternInterpolationUBO), idLineInterpolationUBO},
    UniformBlockInfo{true, true, sizeof(LinePatternTilePropertiesUBO), idLineTilePropertiesUBO},
    UniformBlockInfo{true, true, sizeof(LineEvaluatedPropsUBO), idLineEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::UShort4, idLinePatternFromVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::UShort4, idLinePatternToVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idLineImageTexture},
};

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, true, sizeof(LineSDFDrawableUBO), idLineDrawableUBO},
    UniformBlockInfo{true, false, sizeof(LineSDFInterpolationUBO), idLineInterpolationUBO},
    UniformBlockInfo{true, true, sizeof(LineEvaluatedPropsUBO), idLineEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float2, idLineFloorWidthVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idLineImageTexture},
};

} // namespace shaders
} // namespace mbgl
