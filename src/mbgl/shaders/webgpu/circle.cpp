#include <mbgl/shaders/webgpu/circle.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using CircleShaderSource = ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 8> CircleShaderSource::attributes = {
    AttributeInfo{circleUBOCount + 0, gfx::AttributeDataType::Short2, idCirclePosVertexAttribute},
    AttributeInfo{circleUBOCount + 1, gfx::AttributeDataType::Float4, idCircleColorVertexAttribute},
    AttributeInfo{circleUBOCount + 2, gfx::AttributeDataType::Float2, idCircleRadiusVertexAttribute},
    AttributeInfo{circleUBOCount + 3, gfx::AttributeDataType::Float2, idCircleBlurVertexAttribute},
    AttributeInfo{circleUBOCount + 4, gfx::AttributeDataType::Float2, idCircleOpacityVertexAttribute},
    AttributeInfo{circleUBOCount + 5, gfx::AttributeDataType::Float4, idCircleStrokeColorVertexAttribute},
    AttributeInfo{circleUBOCount + 6, gfx::AttributeDataType::Float2, idCircleStrokeWidthVertexAttribute},
    AttributeInfo{circleUBOCount + 7, gfx::AttributeDataType::Float2, idCircleStrokeOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> CircleShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl