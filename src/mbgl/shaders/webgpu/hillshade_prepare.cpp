#include <mbgl/shaders/webgpu/hillshade_prepare.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using HillshadePrepareShaderSource = ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> HillshadePrepareShaderSource::attributes = {
    AttributeInfo{hillshadePrepareUBOCount + 0, gfx::AttributeDataType::Float2, idHillshadePosVertexAttribute},
    AttributeInfo{hillshadePrepareUBOCount + 1, gfx::AttributeDataType::Float2, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> HillshadePrepareShaderSource::textures = {
    TextureInfo{0, idHillshadeImageTexture}
};

} // namespace shaders
} // namespace mbgl