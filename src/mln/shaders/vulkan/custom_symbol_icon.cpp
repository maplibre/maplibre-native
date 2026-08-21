#include <mln/shaders/vulkan/custom_symbol_icon.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/custom_drawable_layer_ubo.hpp>

namespace mln {
namespace shaders {

using CustomSymbolIconShaderSource = ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 2> CustomSymbolIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idCustomSymbolPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idCustomSymbolTexVertexAttribute},
};
const std::array<TextureInfo, 1> CustomSymbolIconShaderSource::textures = {
    TextureInfo{0, idCustomSymbolImageTexture},
};

} // namespace shaders
} // namespace mln
