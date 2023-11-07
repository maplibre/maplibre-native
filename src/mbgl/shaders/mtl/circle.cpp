// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/circle.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::reflectionData = {
    "CircleShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Float2, 1, "a_color"},
        AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_radius"},
        AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_blur"},
        AttributeInfo{4, gfx::AttributeDataType::Float2, 1, "a_opacity"},
        AttributeInfo{5, gfx::AttributeDataType::Float4, 1, "a_stroke_color"},
        AttributeInfo{6, gfx::AttributeDataType::Float2, 1, "a_stroke_width"},
        AttributeInfo{7, gfx::AttributeDataType::Float2, 1, "a_stroke_opacity"},
    },
    {
        UniformBlockInfo{8, true, false, sizeof(CircleDrawableUBO), "CircleDrawableUBO"},
        UniformBlockInfo{9, true, true, sizeof(CirclePaintParamsUBO), "CirclePaintParamsUBO"},
        UniformBlockInfo{10, true, true, sizeof(CircleEvaluatedPropsUBO), "CircleEvaluatedPropsUBO"},
        UniformBlockInfo{11, true, false, sizeof(CircleInterpolateUBO), "CircleInterpolateUBO"},
        UniformBlockInfo{12, true, true, sizeof(CirclePermutationUBO), "CirclePermutationUBO"},
        UniformBlockInfo{13, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
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
