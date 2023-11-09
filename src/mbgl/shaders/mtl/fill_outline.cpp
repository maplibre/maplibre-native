// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/fill_outline.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::reflectionData = {
    "FillOutlineShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Float2, 1, "a_outline_color"},
        AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_opacity"},
        AttributeInfo{8, gfx::AttributeDataType::Float2, 1, "a_color"},
    },
    {
        UniformBlockInfo{3, true, false, sizeof(FillOutlineDrawableUBO), "FillOutlineDrawableUBO"},
        UniformBlockInfo{4, true, true, sizeof(FillOutlineEvaluatedPropsUBO), "FillOutlineEvaluatedPropsUBO"},
        UniformBlockInfo{5, true, false, sizeof(FillOutlineInterpolateUBO), "FillOutlineInterpolateUBO"},
        UniformBlockInfo{6, true, true, sizeof(FillOutlinePermutationUBO), "FillOutlinePermutationUBO"},
        UniformBlockInfo{7, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
    },
    {

    },
    {
        BuiltIn::Prelude,
    },
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
