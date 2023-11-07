// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/symbol_text_and_icon.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal>::reflectionData = {
    "SymbolTextAndIconShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short4, 1, "a_pos_offset"},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, 1, "a_data"},
        AttributeInfo{2, gfx::AttributeDataType::Float3, 1, "a_projected_pos"},
        AttributeInfo{3, gfx::AttributeDataType::Float, 1, "a_fade_opacity"},
        AttributeInfo{4, gfx::AttributeDataType::Float4, 1, "a_fill_color"},
        AttributeInfo{5, gfx::AttributeDataType::Float4, 1, "a_halo_color"},
        AttributeInfo{6, gfx::AttributeDataType::Float, 1, "a_opacity"},
        AttributeInfo{7, gfx::AttributeDataType::Float, 1, "a_halo_width"},
        AttributeInfo{8, gfx::AttributeDataType::Float, 1, "a_halo_blur"},
    },
    {
        UniformBlockInfo{9, true, true, sizeof(SymbolDrawableUBO), "SymbolDrawableUBO"},
        UniformBlockInfo{10, true, false, sizeof(SymbolDrawablePaintUBO), "SymbolDrawablePaintUBO"},
        UniformBlockInfo{11, true, true, sizeof(SymbolDrawableTilePropsUBO), "SymbolDrawableTilePropsUBO"},
        UniformBlockInfo{12, true, false, sizeof(SymbolDrawableInterpolateUBO), "SymbolDrawableInterpolateUBO"},
        UniformBlockInfo{13, true, true, sizeof(SymbolPermutationUBO), "SymbolPermutationUBO"},
        UniformBlockInfo{14, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
    },
    {
        TextureInfo{0, "u_texture"},
        TextureInfo{1, "u_texture_icon"},
    },
    {
        BuiltIn::Prelude,
    },
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
