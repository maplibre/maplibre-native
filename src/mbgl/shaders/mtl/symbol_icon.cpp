// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/symbol_icon.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>::reflectionData = {
    "SymbolIconShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short4, 1, "a_pos_offset"},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, 1, "a_data"},
        AttributeInfo{2, gfx::AttributeDataType::Short4, 1, "a_pixeloffset"},
        AttributeInfo{3, gfx::AttributeDataType::Float3, 1, "a_projected_pos"},
        AttributeInfo{4, gfx::AttributeDataType::Float, 1, "a_fade_opacity"},
        AttributeInfo{5, gfx::AttributeDataType::Float, 1, "a_opacity"},
    },
    {
        UniformBlockInfo{8, true, true, sizeof(SymbolDrawableUBO), "SymbolDrawableUBO"},
        UniformBlockInfo{9, true, true, sizeof(SymbolDrawablePaintUBO), "SymbolDrawablePaintUBO"},
        UniformBlockInfo{10, true, false, sizeof(SymbolDrawableTilePropsUBO), "SymbolDrawableTilePropsUBO"},
        UniformBlockInfo{11, true, false, sizeof(SymbolDrawableInterpolateUBO), "SymbolDrawableInterpolateUBO"},
        UniformBlockInfo{12, true, true, sizeof(SymbolPermutationUBO), "SymbolPermutationUBO"},
        UniformBlockInfo{13, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
    },
    {
        TextureInfo{0, "u_texture"},
    },
    {
        BuiltIn::Prelude,
    },
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
