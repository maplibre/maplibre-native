#include <mbgl/shaders/mtl/line.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos_normal"},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, "a_data"},
    AttributeInfo{2, gfx::AttributeDataType::Float4, "a_color"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, "a_blur"},
    AttributeInfo{4, gfx::AttributeDataType::Float2, "a_opacity"},
    AttributeInfo{5, gfx::AttributeDataType::Float2, "a_gapwidth"},
    AttributeInfo{6, gfx::AttributeDataType::Float2, "a_offset"},
    AttributeInfo{7, gfx::AttributeDataType::Float2, "a_width"},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{8, true, false, sizeof(LineDynamicUBO), idLineDynamicUBO},
    UniformBlockInfo{9, true, true, sizeof(LineUBO), idLineUBO},
    UniformBlockInfo{10, true, true, sizeof(LinePropertiesUBO), idLinePropertiesUBO},
    UniformBlockInfo{11, true, false, sizeof(LineInterpolationUBO), idLineInterpolationUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos_normal"},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, "a_data"},
    AttributeInfo{2, gfx::AttributeDataType::Float2, "a_blur"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, "a_opacity"},
    AttributeInfo{4, gfx::AttributeDataType::Float2, "a_gapwidth"},
    AttributeInfo{5, gfx::AttributeDataType::Float2, "a_offset"},
    AttributeInfo{6, gfx::AttributeDataType::Float2, "a_width"},
    AttributeInfo{7, gfx::AttributeDataType::UShort4, "a_pattern_from"},
    AttributeInfo{8, gfx::AttributeDataType::UShort4, "a_pattern_to"},
};
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{9, true, false, sizeof(LineDynamicUBO), idLinePatternDynamicUBO},
    UniformBlockInfo{10, true, true, sizeof(LinePatternUBO), idLinePatternUBO},
    UniformBlockInfo{11, true, true, sizeof(LinePatternPropertiesUBO), idLinePatternPropertiesUBO},
    UniformBlockInfo{12, true, false, sizeof(LinePatternInterpolationUBO), idLinePatternInterpolationUBO},
    UniformBlockInfo{13, true, true, sizeof(LinePatternTilePropertiesUBO), idLinePatternTilePropertiesUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_image"},
};

const std::array<AttributeInfo, 9> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos_normal"},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, "a_data"},
    AttributeInfo{2, gfx::AttributeDataType::Float4, "a_color"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, "a_blur"},
    AttributeInfo{4, gfx::AttributeDataType::Float2, "a_opacity"},
    AttributeInfo{5, gfx::AttributeDataType::Float2, "a_gapwidth"},
    AttributeInfo{6, gfx::AttributeDataType::Float2, "a_offset"},
    AttributeInfo{7, gfx::AttributeDataType::Float2, "a_width"},
    AttributeInfo{8, gfx::AttributeDataType::Float2, "a_floorwidth"},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{9, true, false, sizeof(LineDynamicUBO), idLineSDFDynamicUBO},
    UniformBlockInfo{10, true, true, sizeof(LineSDFUBO), idLineSDFUBO},
    UniformBlockInfo{11, true, true, sizeof(LineSDFPropertiesUBO), idLineSDFPropertiesUBO},
    UniformBlockInfo{12, true, false, sizeof(LineSDFInterpolationUBO), idLineSDFInterpolationUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_image"},
};

const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos_normal"},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, "a_data"},
};
const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{2, true, true, sizeof(LineBasicUBO), idLineBasicUBO},
    UniformBlockInfo{3, true, true, sizeof(LineBasicPropertiesUBO), idLineBasicPropertiesUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
