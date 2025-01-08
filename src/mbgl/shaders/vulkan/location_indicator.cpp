#include <mbgl/shaders/vulkan/location_indicator.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/location_indicator_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Location indicator

using LocationIndicatorShaderSource = ShaderSource<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> LocationIndicatorShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idCommonPosVertexAttribute},
};
const std::array<TextureInfo, 0> LocationIndicatorShaderSource::textures = {};

//
// Location indicator textured

using LocationIndicatorShaderTexturedSource =
    ShaderSource<BuiltIn::LocationIndicatorTexturedShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 2> LocationIndicatorShaderTexturedSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idCommonPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idCommonTexVertexAttribute},
};
const std::array<TextureInfo, 1> LocationIndicatorShaderTexturedSource::textures = {
    TextureInfo{0, idCommonTexture},
};

} // namespace shaders
} // namespace mbgl
