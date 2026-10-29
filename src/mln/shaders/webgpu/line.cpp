#include <mln/shaders/webgpu/line.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/line_layer_ubo.hpp>

namespace mln {
namespace shaders {

using LineShaderSource = ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> LineShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UInt, idLineColorVertexAttribute},
};
const std::array<TextureInfo, 0> LineShaderSource::textures = {};

// Line Gradient
using LineGradientShaderSource = ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> LineGradientShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UInt, idLineColorVertexAttribute},
};
const std::array<TextureInfo, 1> LineGradientShaderSource::textures = {TextureInfo{0, idLineImageTexture}};

// Line Pattern
using LinePatternShaderSource = ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> LinePatternShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UInt, idLineColorVertexAttribute},
};
const std::array<TextureInfo, 1> LinePatternShaderSource::textures = {TextureInfo{0, idLineImageTexture}};

// Line SDF
using LineSDFShaderSource = ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> LineSDFShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UInt, idLineColorVertexAttribute},
};
const std::array<TextureInfo, 1> LineSDFShaderSource::textures = {TextureInfo{0, idLineImageTexture}};

// Line gradient SDF
using LineGradientSDFShaderSource = ShaderSource<BuiltIn::LineGradientSDFShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> LineGradientSDFShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UInt, idLineColorVertexAttribute},
};
// A texture at location L binds its sampler at L and the texture at L + 1, so the dash atlas sits at 2.
const std::array<TextureInfo, 2> LineGradientSDFShaderSource::textures = {TextureInfo{0, idLineImageTexture},
                                                                          TextureInfo{2, idLineDashTexture}};

} // namespace shaders
} // namespace mln
