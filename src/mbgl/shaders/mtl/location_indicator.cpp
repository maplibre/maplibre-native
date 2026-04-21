#include <mbgl/shaders/mtl/location_indicator.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using LocationIndicatorShaderSource = ShaderSource<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> LocationIndicatorShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, locationIndicatorUBOCount + 0, idLocationIndicatorPosVertexAttribute}
};

using LocationIndicatorTexturedShaderSource =
    ShaderSource<BuiltIn::LocationIndicatorTexturedShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> LocationIndicatorTexturedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, locationIndicatorUBOCount + 0, idLocationIndicatorPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, locationIndicatorUBOCount + 1, idLocationIndicatorTexVertexAttribute},
};

const std::array<TextureInfo, 1> LocationIndicatorTexturedShaderSource::textures = {
    TextureInfo{0, idLocationIndicatorTexture},
};

} // namespace shaders
} // namespace mbgl
