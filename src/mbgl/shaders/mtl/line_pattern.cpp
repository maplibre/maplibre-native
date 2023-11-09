// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/line_pattern.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::reflectionData = {
    "LinePatternShader",
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
        AttributeInfo{7, gfx::AttributeDataType::UShort4, 1, "a_pattern_from"},
        AttributeInfo{8, gfx::AttributeDataType::UShort4, 1, "a_pattern_to"},
    },
    {
        UniformBlockInfo{9, true, true, sizeof(LinePatternUBO), "LinePatternUBO"},
        UniformBlockInfo{10, true, false, sizeof(LinePatternPropertiesUBO), "LinePatternPropertiesUBO"},
        UniformBlockInfo{11, true, false, sizeof(LinePatternInterpolationUBO), "LinePatternInterpolationUBO"},
        UniformBlockInfo{12, true, false, sizeof(LinePatternTilePropertiesUBO), "LinePatternTilePropertiesUBO"},
        UniformBlockInfo{13, true, true, sizeof(LinePermutationUBO), "LinePermutationUBO"},
        UniformBlockInfo{14, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
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
