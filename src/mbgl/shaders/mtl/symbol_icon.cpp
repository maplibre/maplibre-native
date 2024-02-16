#include <mbgl/shaders/mtl/symbol_icon.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 6> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short4, "a_pos_offset"},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, "a_data"},
    AttributeInfo{2, gfx::AttributeDataType::Short4, "a_pixeloffset"},
    AttributeInfo{3, gfx::AttributeDataType::Float3, "a_projected_pos"},
    AttributeInfo{4, gfx::AttributeDataType::Float, "a_fade_opacity"},

    // sometimes uniforms
    AttributeInfo{5, gfx::AttributeDataType::Float, "a_opacity"},
};
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{6, true, true, sizeof(SymbolDrawableUBO), idSymbolDrawableUBO},
    UniformBlockInfo{7, true, false, sizeof(SymbolDynamicUBO), idSymbolDynamicUBO},
    UniformBlockInfo{8, true, true, sizeof(SymbolDrawablePaintUBO), idSymbolDrawablePaintUBO},
    UniformBlockInfo{9, true, false, sizeof(SymbolDrawableTilePropsUBO), idSymbolDrawableTilePropsUBO},
    UniformBlockInfo{10, true, false, sizeof(SymbolDrawableInterpolateUBO), idSymbolDrawableInterpolateUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

} // namespace shaders
} // namespace mbgl
