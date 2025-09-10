#include <mbgl/shaders/webgpu/custom_geometry.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/custom_drawable_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using CustomSymbolIconShaderSource = ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> CustomSymbolIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float3, idCustomSymbolPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idCustomSymbolTexCoordVertexAttribute},
};
const std::array<TextureInfo, 1> CustomSymbolIconShaderSource::textures = {
    TextureInfo{0, idCustomSymbolIconTexture}
};

} // namespace shaders
} // namespace mbgl