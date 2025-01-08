#include <mbgl/shaders/mtl/background.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Background

using BackgroundShaderSource = ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> BackgroundShaderSource::attributes = {
    AttributeInfo{backgroundUBOCount + 0, gfx::AttributeDataType::Float3, idBackgroundPosVertexAttribute},
};
const std::array<TextureInfo, 0> BackgroundShaderSource::textures = {};

//
// Background pattern

using BackgroundPatternShaderSource = ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> BackgroundPatternShaderSource::attributes = {
    AttributeInfo{backgroundUBOCount + 0, gfx::AttributeDataType::Float3, idBackgroundPosVertexAttribute},
};
const std::array<TextureInfo, 1> BackgroundPatternShaderSource::textures = {TextureInfo{0, idBackgroundImageTexture}};

} // namespace shaders
} // namespace mbgl
