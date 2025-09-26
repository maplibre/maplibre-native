#include <mbgl/shaders/webgpu/hillshade.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using HillshadeShaderSource = ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> HillshadeShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Short2, idHillshadePosVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Short2, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> HillshadeShaderSource::textures = {TextureInfo{0, idHillshadeImageTexture}};

} // namespace shaders
} // namespace mbgl
