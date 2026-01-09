#include <mbgl/shaders/webgpu/location_indicator.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/location_indicator_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Location indicator

using LocationIndicatorShaderSource = ShaderSource<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> LocationIndicatorShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLocationIndicatorPosVertexAttribute},
};
const std::array<TextureInfo, 0> LocationIndicatorShaderSource::textures = {};

//
// Location indicator textured

using LocationIndicatorTexturedShaderSource =
    ShaderSource<BuiltIn::LocationIndicatorTexturedShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> LocationIndicatorTexturedShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLocationIndicatorPosVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLocationIndicatorTexVertexAttribute},
};
const std::array<TextureInfo, 1> LocationIndicatorTexturedShaderSource::textures = {
    TextureInfo{0, idLocationIndicatorTexture}};

} // namespace shaders
} // namespace mbgl
