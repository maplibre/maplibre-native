// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/line_gradient.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal>::reflectionData = {
    "LineGradientShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos_normal"},
        AttributeInfo{1, gfx::AttributeDataType::UByte4, 1, "a_data"},
        AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_blur"},
        AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_opacity"},
        AttributeInfo{4, gfx::AttributeDataType::Float2, 1, "a_gapwidth"},
        AttributeInfo{5, gfx::AttributeDataType::Float2, 1, "a_offset"},
        AttributeInfo{6, gfx::AttributeDataType::Float2, 1, "a_width"},
    },
    {
        UniformBlockInfo{7, true, true, sizeof(LineGradientUBO), "LineGradientUBO"},
        UniformBlockInfo{8, true, false, sizeof(LineGradientPropertiesUBO), "LineGradientPropertiesUBO"},
        UniformBlockInfo{9, true, false, sizeof(LineGradientInterpolationUBO), "LineGradientInterpolationUBO"},
        UniformBlockInfo{10, true, true, sizeof(LinePermutationUBO), "LinePermutationUBO"},
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
