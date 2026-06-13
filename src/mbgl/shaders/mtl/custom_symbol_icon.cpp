#include <mbgl/shaders/mtl/custom_symbol_icon.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using CustomSymbolIconShaderSource = ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> CustomSymbolIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, customSymbolUBOCount + 0, idCustomSymbolPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, customSymbolUBOCount + 0, idCustomSymbolTexVertexAttribute},
};
const std::array<TextureInfo, 1> CustomSymbolIconShaderSource::textures = {
    TextureInfo{0, idCustomSymbolImageTexture},
};

} // namespace shaders
} // namespace mbgl
