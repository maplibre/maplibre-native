#include <mbgl/shaders/mtl/line.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Line

using LineShaderSource = ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 8> LineShaderSource::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{lineUBOCount + 2, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{lineUBOCount + 3, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{lineUBOCount + 4, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{lineUBOCount + 5, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{lineUBOCount + 6, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{lineUBOCount + 7, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<TextureInfo, 0> LineShaderSource::textures = {};

//
// Line gradient

using LineGradientShaderSource = ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 7> LineGradientShaderSource::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{lineUBOCount + 2, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{lineUBOCount + 3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{lineUBOCount + 4, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{lineUBOCount + 5, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{lineUBOCount + 6, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<TextureInfo, 1> LineGradientShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
};

//
// Line pattern

using LinePatternShaderSource = ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 9> LinePatternShaderSource::attributes = {
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
const std::array<TextureInfo, 1> LinePatternShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
};

//
// Line SDF

using LineSDFShaderSource = ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 9> LineSDFShaderSource::attributes = {
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
const std::array<TextureInfo, 1> LineSDFShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
};

} // namespace shaders
} // namespace mbgl
