// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/fill_outline_pattern.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::reflectionData = {
    "FillOutlinePatternShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, 1, "a_pattern_from"},
        AttributeInfo{2, gfx::AttributeDataType::UShort4, 1, "a_pattern_to"},
        AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_opacity"},
    },
    {
        UniformBlockInfo{4, true, true, sizeof(FillOutlinePatternDrawableUBO), "FillOutlinePatternDrawableUBO"},
        UniformBlockInfo{5, true, false, sizeof(FillOutlinePatternTilePropsUBO), "FillOutlinePatternTilePropsUBO"},
        UniformBlockInfo{6, true, true, sizeof(FillOutlinePatternEvaluatedPropsUBO), "FillOutlinePatternEvaluatedPropsUBO"},
        UniformBlockInfo{7, true, false, sizeof(FillOutlinePatternInterpolateUBO), "FillOutlinePatternInterpolateUBO"},
        UniformBlockInfo{8, true, true, sizeof(FillOutlinePatternPermutationUBO), "FillOutlinePatternPermutationUBO"},
        UniformBlockInfo{9, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
    },
    {
        TextureInfo{0, "u_image"},
    },
    {
        BuiltIn::Prelude,
    },
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
