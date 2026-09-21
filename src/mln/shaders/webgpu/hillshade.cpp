#include <mln/shaders/webgpu/hillshade.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/hillshade_layer_ubo.hpp>

namespace mln {
namespace shaders {

using HillshadeShaderSource = ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> HillshadeShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Short2, idHillshadePosVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Short2, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> HillshadeShaderSource::textures = {TextureInfo{0, idHillshadeImageTexture}};

} // namespace shaders
} // namespace mln
