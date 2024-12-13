#include <mbgl/shaders/vulkan/line.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Line

using LineShaderSource = ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 8> LineShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<TextureInfo, 0> LineShaderSource::textures = {};

//
// Line gradient

using LineGradientShaderSource = ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 7> LineGradientShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<TextureInfo, 1> LineGradientShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
};

//
// Line pattern

using LinePatternShaderSource = ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 9> LinePatternShaderSource::attributes = {
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
const std::array<TextureInfo, 1> LinePatternShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
};

//
// Line SDF

using LineSDFShaderSource = ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 9> LineSDFShaderSource::attributes = {
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
const std::array<TextureInfo, 1> LineSDFShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
};

} // namespace shaders
} // namespace mbgl
