#include <mbgl/shaders/mtl/line.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{8, true, false, sizeof(LineDynamicUBO), idLineDynamicUBO},
    UniformBlockInfo{9, true, true, sizeof(LineUBO), idLineUBO},
    UniformBlockInfo{10, true, true, sizeof(LinePropertiesUBO), idLinePropertiesUBO},
    UniformBlockInfo{11, true, false, sizeof(LineInterpolationUBO), idLineInterpolationUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::attributes = {
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
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{9, true, false, sizeof(LineDynamicUBO), idLinePatternDynamicUBO},
    UniformBlockInfo{10, true, true, sizeof(LinePatternUBO), idLinePatternUBO},
    UniformBlockInfo{11, true, true, sizeof(LinePatternPropertiesUBO), idLinePatternPropertiesUBO},
    UniformBlockInfo{12, true, false, sizeof(LinePatternInterpolationUBO), idLinePatternInterpolationUBO},
    UniformBlockInfo{13, true, true, sizeof(LinePatternTilePropertiesUBO), idLinePatternTilePropertiesUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idLineImageTexture},
};

const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::attributes = {
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
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{9, true, false, sizeof(LineDynamicUBO), idLineSDFDynamicUBO},
    UniformBlockInfo{10, true, true, sizeof(LineSDFUBO), idLineSDFUBO},
    UniformBlockInfo{11, true, true, sizeof(LineSDFPropertiesUBO), idLineSDFPropertiesUBO},
    UniformBlockInfo{12, true, false, sizeof(LineSDFInterpolationUBO), idLineSDFInterpolationUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idLineImageTexture},
};

const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
};
const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{2, true, true, sizeof(LineBasicUBO), idLineBasicUBO},
    UniformBlockInfo{3, true, true, sizeof(LineBasicPropertiesUBO), idLineBasicPropertiesUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
