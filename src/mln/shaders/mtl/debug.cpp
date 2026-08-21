#include <mln/shaders/mtl/debug.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

using DebugShaderSource = ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> DebugShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, debugUBOCount + 0, idDebugPosVertexAttribute},
};
const std::array<TextureInfo, 1> DebugShaderSource::textures = {
    TextureInfo{0, idDebugOverlayTexture},
};

} // namespace shaders
} // namespace mln
