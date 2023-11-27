#include <mbgl/shaders/mtl/line.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos_normal"},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, 1, "a_data"},
    AttributeInfo{2, gfx::AttributeDataType::Float4, 1, "a_color"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_blur"},
    AttributeInfo{4, gfx::AttributeDataType::Float2, 1, "a_opacity"},
    AttributeInfo{5, gfx::AttributeDataType::Float2, 1, "a_gapwidth"},
    AttributeInfo{6, gfx::AttributeDataType::Float2, 1, "a_offset"},
    AttributeInfo{7, gfx::AttributeDataType::Float2, 1, "a_width"},
};
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{8, true, true, sizeof(LineUBO), "LineUBO"},
    UniformBlockInfo{9, true, false, sizeof(LinePropertiesUBO), "LinePropertiesUBO"},
    UniformBlockInfo{10, true, false, sizeof(LineInterpolationUBO), "LineInterpolationUBO"},
    UniformBlockInfo{11, true, true, sizeof(LinePermutationUBO), "LinePermutationUBO"},
    UniformBlockInfo{12, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos_normal"},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, 1, "a_data"},
    AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_blur"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_opacity"},
    AttributeInfo{4, gfx::AttributeDataType::Float2, 1, "a_gapwidth"},
    AttributeInfo{5, gfx::AttributeDataType::Float2, 1, "a_offset"},
    AttributeInfo{6, gfx::AttributeDataType::Float2, 1, "a_width"},
    AttributeInfo{7, gfx::AttributeDataType::UShort4, 1, "a_pattern_from"},
    AttributeInfo{8, gfx::AttributeDataType::UShort4, 1, "a_pattern_to"},
};
const std::array<UniformBlockInfo, 6> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{9, true, true, sizeof(LinePatternUBO), "LinePatternUBO"},
    UniformBlockInfo{10, true, false, sizeof(LinePatternPropertiesUBO), "LinePatternPropertiesUBO"},
    UniformBlockInfo{11, true, false, sizeof(LinePatternInterpolationUBO), "LinePatternInterpolationUBO"},
    UniformBlockInfo{12, true, false, sizeof(LinePatternTilePropertiesUBO), "LinePatternTilePropertiesUBO"},
    UniformBlockInfo{13, true, true, sizeof(LinePermutationUBO), "LinePermutationUBO"},
    UniformBlockInfo{14, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_image"},
};

const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos_normal"},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, 1, "a_data"},
    AttributeInfo{2, gfx::AttributeDataType::Float4, 1, "a_color"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_blur"},
    AttributeInfo{4, gfx::AttributeDataType::Float2, 1, "a_opacity"},
    AttributeInfo{5, gfx::AttributeDataType::Float2, 1, "a_gapwidth"},
    AttributeInfo{6, gfx::AttributeDataType::Float2, 1, "a_offset"},
    AttributeInfo{7, gfx::AttributeDataType::Float2, 1, "a_width"},
    AttributeInfo{8, gfx::AttributeDataType::Float2, 1, "a_floorwidth"},
};
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{9, true, true, sizeof(LineSDFUBO), "LineSDFUBO"},
    UniformBlockInfo{10, true, false, sizeof(LineSDFPropertiesUBO), "LineSDFPropertiesUBO"},
    UniformBlockInfo{11, true, false, sizeof(LineSDFInterpolationUBO), "LineSDFInterpolationUBO"},
    UniformBlockInfo{12, true, true, sizeof(LinePermutationUBO), "LinePermutationUBO"},
    UniformBlockInfo{13, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_image"},
};

const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos_normal"},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, 1, "a_data"},
};
const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{2, true, true, sizeof(LineBasicUBO), "LineBasicUBO"},
    UniformBlockInfo{3, true, false, sizeof(LineBasicPropertiesUBO), "LineBasicPropertiesUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
