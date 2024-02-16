#include <mbgl/shaders/mtl/custom_symbol_icon.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 2>
    ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Metal>::attributes = {
        // always attributes
        AttributeInfo{0, gfx::AttributeDataType::Float2, idCustomSymbolPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::Float2, idCustomSymbolTexVertexAttribute},
};
const std::array<UniformBlockInfo, 2>
    ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{2, true, false, sizeof(CustomSymbolIconDrawableUBO), idCustomSymbolIconDrawableUBO},
        UniformBlockInfo{3, true, false, sizeof(CustomSymbolIconParametersUBO), idCustomSymbolIconParametersUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idCustomSymbolIconTexture},
};

} // namespace shaders
} // namespace mbgl
