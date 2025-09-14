#include <mbgl/shaders/webgpu/debug.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/debug_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using DebugShaderSource = ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> DebugShaderSource::attributes = {
    AttributeInfo{debugUBOCount + 0, gfx::AttributeDataType::Float2, idDebugPosVertexAttribute},
};
const std::array<TextureInfo, 1> DebugShaderSource::textures = {
    TextureInfo{0, idDebugOverlayTexture}
};

} // namespace shaders
} // namespace mbgl