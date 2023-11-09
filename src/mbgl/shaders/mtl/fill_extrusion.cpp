// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/fill_extrusion.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>::reflectionData = {
    "FillExtrusionShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Short4, 1, "a_normal_ed"},
        AttributeInfo{2, gfx::AttributeDataType::Float4, 1, "a_color"},
        AttributeInfo{3, gfx::AttributeDataType::Float, 1, "a_base"},
        AttributeInfo{4, gfx::AttributeDataType::Float, 1, "a_height"},
    },
    {
        UniformBlockInfo{5, true, false, sizeof(FillExtrusionDrawableUBO), "FillExtrusionDrawableUBO"},
        UniformBlockInfo{6, true, false, sizeof(FillExtrusionDrawablePropsUBO), "FillExtrusionDrawablePropsUBO"},
        UniformBlockInfo{7, true, false, sizeof(FillExtrusionInterpolateUBO), "FillExtrusionInterpolateUBO"},
        UniformBlockInfo{8, true, false, sizeof(FillExtrusionPermutationUBO), "FillExtrusionPermutationUBO"},
        UniformBlockInfo{9, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
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
