// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/fill.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::reflectionData = {
    "FillShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Float2, 1, "a_color"},
        AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_opacity"},
        AttributeInfo{8, gfx::AttributeDataType::Float2, 1, "a_outline_color"},
    },
    {
        UniformBlockInfo{3, true, false, sizeof(FillDrawableUBO), "FillDrawableUBO"},
        UniformBlockInfo{4, true, true, sizeof(FillEvaluatedPropsUBO), "FillEvaluatedPropsUBO"},
        UniformBlockInfo{5, true, false, sizeof(FillInterpolateUBO), "FillInterpolateUBO"},
        UniformBlockInfo{6, true, true, sizeof(FillPermutationUBO), "FillPermutationUBO"},
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
