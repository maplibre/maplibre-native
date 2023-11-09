// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/line_sdf.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::reflectionData = {
    "LineSDFShader",
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
        AttributeInfo{8, gfx::AttributeDataType::Float2, 1, "a_floorwidth"},
    },
    {
        UniformBlockInfo{9, true, true, sizeof(LineSDFUBO), "LineSDFUBO"},
        UniformBlockInfo{10, true, false, sizeof(LineSDFPropertiesUBO), "LineSDFPropertiesUBO"},
        UniformBlockInfo{11, true, false, sizeof(LineSDFInterpolationUBO), "LineSDFInterpolationUBO"},
        UniformBlockInfo{12, true, true, sizeof(LinePermutationUBO), "LinePermutationUBO"},
        UniformBlockInfo{13, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
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
