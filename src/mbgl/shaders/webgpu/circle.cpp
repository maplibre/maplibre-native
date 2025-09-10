#include <mbgl/shaders/webgpu/circle.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using CircleShaderSource = ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> CircleShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idCirclePosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idCircleExtrudeVertexAttribute},
};
const std::array<TextureInfo, 0> CircleShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl