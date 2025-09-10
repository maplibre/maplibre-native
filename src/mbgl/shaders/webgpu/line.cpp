#include <mbgl/shaders/webgpu/line.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using LineShaderSource = ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> LineShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float4, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 0> LineShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl