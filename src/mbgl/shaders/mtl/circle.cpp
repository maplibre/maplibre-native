#include <mbgl/shaders/mtl/circle.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using CircleShaderSource = ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 8> CircleShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, circleUBOCount + 0, idCirclePosVertexAttribute},

    // Data driven
    AttributeInfo{1, gfx::AttributeDataType::Float4, circleUBOCount + 1, idCircleColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, circleUBOCount + 1, idCircleRadiusVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, circleUBOCount + 1, idCircleBlurVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, circleUBOCount + 1, idCircleOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float4, circleUBOCount + 1, idCircleStrokeColorVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, circleUBOCount + 1, idCircleStrokeWidthVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, circleUBOCount + 1, idCircleStrokeOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> CircleShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
