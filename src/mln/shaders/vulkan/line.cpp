#include <mln/shaders/vulkan/line.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/line_layer_ubo.hpp>

namespace mln {
namespace shaders {

//
// Line

using LineShaderSource = ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 8> LineShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},

    // sometimes uniforms
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
// Line intersection

using LineIntersectionShaderSource = ShaderSource<BuiltIn::LineIntersectionShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 7> LineIntersectionShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},

    // sometimes uniforms
    AttributeInfo{1, gfx::AttributeDataType::Float4, idLineColorVertexAttribute, idLineDataVertexDrawableUBO},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute, idLineDataVertexDrawableUBO},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute, idLineDataVertexDrawableUBO},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute, idLineDataVertexDrawableUBO},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute, idLineDataVertexDrawableUBO},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute, idLineDataVertexDrawableUBO},
};
const std::array<AttributeInfo, 2> LineIntersectionShaderSource::instanceAttributes = {
    // used only in intersection rounded corners
    AttributeInfo{7, gfx::AttributeDataType::UInt3, idLineTriangleVerticesAttribute},
    AttributeInfo{8, gfx::AttributeDataType::UInt2, idLineDataIndicesAttribute},
};
const std::array<TextureInfo, 0> LineIntersectionShaderSource::textures = {};

//
// Line gradient

using LineGradientShaderSource = ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 7> LineGradientShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},

    // sometimes uniforms
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
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},

    // sometimes uniforms
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
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},

    // sometimes uniforms
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

//
// Line gradient SDF

using LineGradientSDFShaderSource = ShaderSource<BuiltIn::LineGradientSDFShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 8> LineGradientSDFShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},

    // sometimes uniforms
    AttributeInfo{2, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineFloorWidthVertexAttribute},
};
const std::array<TextureInfo, 2> LineGradientSDFShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
    TextureInfo{1, idLineDashTexture},
};

} // namespace shaders
} // namespace mln
