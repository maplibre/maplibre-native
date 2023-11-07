// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/heatmap.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Metal>::reflectionData = {
    "HeatmapShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Float2, 1, "a_weight"},
        AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_radius"},
    },
    {
        UniformBlockInfo{3, true, false, sizeof(HeatmapDrawableUBO), "HeatmapDrawableUBO"},
        UniformBlockInfo{4, true, true, sizeof(HeatmapEvaluatedPropsUBO), "HeatmapEvaluatedPropsUBO"},
        UniformBlockInfo{5, true, false, sizeof(HeatmapInterpolateUBO), "HeatmapInterpolateUBO"},
        UniformBlockInfo{6, true, true, sizeof(HeatmapPermutationUBO), "HeatmapPermutationUBO"},
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
