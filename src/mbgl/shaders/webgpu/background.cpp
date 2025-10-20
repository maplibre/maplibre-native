#include <mbgl/shaders/webgpu/background.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Background

using BackgroundShaderSource = ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> BackgroundShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short2, idBackgroundPosVertexAttribute},
};
const std::array<TextureInfo, 0> BackgroundShaderSource::textures = {};

//
// Background pattern

using BackgroundPatternShaderSource = ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> BackgroundPatternShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short2, idBackgroundPosVertexAttribute},
};
const std::array<TextureInfo, 1> BackgroundPatternShaderSource::textures = {TextureInfo{0, idBackgroundImageTexture}};

} // namespace shaders
} // namespace mbgl
