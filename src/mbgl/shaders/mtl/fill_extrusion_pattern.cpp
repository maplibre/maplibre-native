// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/fill_extrusion_pattern.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::reflectionData = {
    "FillExtrusionPatternShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Short4, 1, "a_normal_ed"},
        AttributeInfo{2, gfx::AttributeDataType::Float, 1, "a_base"},
        AttributeInfo{3, gfx::AttributeDataType::Float, 1, "a_height"},
        AttributeInfo{4, gfx::AttributeDataType::UShort4, 1, "a_pattern_from"},
        AttributeInfo{5, gfx::AttributeDataType::UShort4, 1, "a_pattern_to"},
    },
    {
        UniformBlockInfo{6, true, true, sizeof(FillExtrusionDrawableUBO), "FillExtrusionDrawableUBO"},
        UniformBlockInfo{7, true, true, sizeof(FillExtrusionDrawablePropsUBO), "FillExtrusionDrawablePropsUBO"},
        UniformBlockInfo{
            8, true, false, sizeof(FillExtrusionDrawableTilePropsUBO), "FillExtrusionDrawableTilePropsUBO"},
        UniformBlockInfo{9, true, false, sizeof(FillExtrusionInterpolateUBO), "FillExtrusionInterpolateUBO"},
        UniformBlockInfo{10, true, true, sizeof(FillExtrusionPermutationUBO), "FillExtrusionPermutationUBO"},
        UniformBlockInfo{11, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
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
