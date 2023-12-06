#include <mbgl/shaders/mtl/symbol_icon.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 6> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short4, 1, "a_pos_offset"},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, 1, "a_data"},
    AttributeInfo{2, gfx::AttributeDataType::Short4, 1, "a_pixeloffset"},
    AttributeInfo{3, gfx::AttributeDataType::Float3, 1, "a_projected_pos"},
    AttributeInfo{4, gfx::AttributeDataType::Float, 1, "a_fade_opacity"},

    // sometimes uniforms
    AttributeInfo{5, gfx::AttributeDataType::Float, 1, "a_opacity"},
};
const std::array<UniformBlockInfo, 8> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{8, true, false, sizeof(MatrixUBO), "SymbolMatrixUBO"},
    UniformBlockInfo{9, true, true, sizeof(SymbolDrawableUBO), "SymbolDrawableUBO"},
    UniformBlockInfo{10, true, false, sizeof(SymbolDynamicUBO), "SymbolDynamicUBO"},
    UniformBlockInfo{11, true, true, sizeof(SymbolDrawablePaintUBO), "SymbolDrawablePaintUBO"},
    UniformBlockInfo{12, true, false, sizeof(SymbolDrawableTilePropsUBO), "SymbolDrawableTilePropsUBO"},
    UniformBlockInfo{13, true, false, sizeof(SymbolDrawableInterpolateUBO), "SymbolDrawableInterpolateUBO"},
    UniformBlockInfo{14, true, true, sizeof(SymbolPermutationUBO), "SymbolPermutationUBO"},
    UniformBlockInfo{15, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_texture"},
};

} // namespace shaders
} // namespace mbgl
