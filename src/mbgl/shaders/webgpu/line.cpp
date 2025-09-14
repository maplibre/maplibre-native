#include <mbgl/shaders/webgpu/line.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using LineShaderSource = ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> LineShaderSource::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Float4, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::Float4, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 0> LineShaderSource::textures = {};

// Line Gradient
using LineGradientShaderSource = ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> LineGradientShaderSource::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Float4, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::Float4, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 1> LineGradientShaderSource::textures = {
    TextureInfo{0, idLineImageTexture}
};

// Line Pattern
using LinePatternShaderSource = ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> LinePatternShaderSource::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Float4, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::Float4, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 1> LinePatternShaderSource::textures = {
    TextureInfo{0, idLineImageTexture}
};

// Line SDF
using LineSDFShaderSource = ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> LineSDFShaderSource::attributes = {
    AttributeInfo{lineUBOCount + 0, gfx::AttributeDataType::Float4, idLinePosNormalVertexAttribute},
    AttributeInfo{lineUBOCount + 1, gfx::AttributeDataType::Float4, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 1> LineSDFShaderSource::textures = {
    TextureInfo{0, idLineImageTexture}
};

} // namespace shaders
} // namespace mbgl