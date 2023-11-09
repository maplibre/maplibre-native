// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/line.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::reflectionData = {
    "LineShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos_normal"},
        AttributeInfo{1, gfx::AttributeDataType::UByte4, 1, "a_data"},
        AttributeInfo{2, gfx::AttributeDataType::Float4, 1, "a_color"},
        AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_blur"},
        AttributeInfo{4, gfx::AttributeDataType::Float2, 1, "a_opacity"},
        AttributeInfo{5, gfx::AttributeDataType::Float2, 1, "a_gapwidth"},
        AttributeInfo{6, gfx::AttributeDataType::Float2, 1, "a_offset"},
        AttributeInfo{7, gfx::AttributeDataType::Float2, 1, "a_width"},
    },
    {
        UniformBlockInfo{8, true, true, sizeof(LineUBO), "LineUBO"},
        UniformBlockInfo{9, true, false, sizeof(LinePropertiesUBO), "LinePropertiesUBO"},
        UniformBlockInfo{10, true, false, sizeof(LineInterpolationUBO), "LineInterpolationUBO"},
        UniformBlockInfo{11, true, true, sizeof(LinePermutationUBO), "LinePermutationUBO"},
        UniformBlockInfo{12, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
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
