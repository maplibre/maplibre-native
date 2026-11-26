#include <mln/shaders/webgpu/debug.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/debug_layer_ubo.hpp>

namespace mln {
namespace shaders {

using DebugShaderSource = ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> DebugShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Short2, idDebugPosVertexAttribute},
};
const std::array<TextureInfo, 1> DebugShaderSource::textures = {TextureInfo{0, idDebugOverlayTexture}};

} // namespace shaders
} // namespace mln
