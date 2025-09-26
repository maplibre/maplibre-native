#include <mbgl/shaders/webgpu/line.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using LineShaderSource = ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 8> LineShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<TextureInfo, 0> LineShaderSource::textures = {};

// Line Gradient
using LineGradientShaderSource = ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 7> LineGradientShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};
const std::array<TextureInfo, 1> LineGradientShaderSource::textures = {TextureInfo{0, idLineImageTexture}};

// Line Pattern
using LinePatternShaderSource = ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 9> LinePatternShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
    AttributeInfo{11, gfx::AttributeDataType::UShort4, idLinePatternFromVertexAttribute},
    AttributeInfo{12, gfx::AttributeDataType::UShort4, idLinePatternToVertexAttribute},
};
const std::array<TextureInfo, 1> LinePatternShaderSource::textures = {TextureInfo{0, idLineImageTexture}};

// Line SDF
using LineSDFShaderSource = ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 9> LineSDFShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
    AttributeInfo{12, gfx::AttributeDataType::Float2, idLineFloorWidthVertexAttribute},
};
const std::array<TextureInfo, 1> LineSDFShaderSource::textures = {TextureInfo{0, idLineImageTexture}};

} // namespace shaders
} // namespace mbgl
