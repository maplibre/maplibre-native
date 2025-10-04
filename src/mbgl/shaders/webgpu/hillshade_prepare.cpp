#include <mbgl/shaders/webgpu/hillshade_prepare.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using HillshadePrepareShaderSource = ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> HillshadePrepareShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Short2, idHillshadePosVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Short2, idHillshadeTexturePosVertexAttribute},
};
const std::array<TextureInfo, 1> HillshadePrepareShaderSource::textures = {TextureInfo{0, idHillshadeImageTexture}};

} // namespace shaders
} // namespace mbgl
