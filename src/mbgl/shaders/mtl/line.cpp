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
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{8, true, false, sizeof(MatrixUBO), "LineMatrixUBO"},
    UniformBlockInfo{9, true, true, sizeof(LineUBO), "LineUBO"},
    UniformBlockInfo{10, true, true, sizeof(LinePropertiesUBO), "LinePropertiesUBO"},
    UniformBlockInfo{11, true, false, sizeof(LineInterpolationUBO), "LineInterpolationUBO"},
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
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{9, true, false, sizeof(MatrixUBO), "LineMatrixUBO"},
    UniformBlockInfo{10, true, true, sizeof(LinePatternUBO), "LinePatternUBO"},
    UniformBlockInfo{11, true, true, sizeof(LinePatternPropertiesUBO), "LinePatternPropertiesUBO"},
    UniformBlockInfo{12, true, false, sizeof(LinePatternInterpolationUBO), "LinePatternInterpolationUBO"},
    UniformBlockInfo{13, true, true, sizeof(LinePatternTilePropertiesUBO), "LinePatternTilePropertiesUBO"},
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
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{9, true, false, sizeof(MatrixUBO), "LineMatrixUBO"},
    UniformBlockInfo{10, true, true, sizeof(LineSDFUBO), "LineSDFUBO"},
    UniformBlockInfo{11, true, true, sizeof(LineSDFPropertiesUBO), "LineSDFPropertiesUBO"},
    UniformBlockInfo{12, true, false, sizeof(LineSDFInterpolationUBO), "LineSDFInterpolationUBO"},
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
    UniformBlockInfo{3, true, true, sizeof(LineBasicPropertiesUBO), "LineBasicPropertiesUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
