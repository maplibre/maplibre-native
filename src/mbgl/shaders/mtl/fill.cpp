#include <mbgl/shaders/mtl/fill.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Float4, 1, "a_color"},
    AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_opacity"},
};
const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{3, true, false, sizeof(FillDrawableUBO), "FillDrawableUBO"},
    UniformBlockInfo{4, true, true, sizeof(FillEvaluatedPropsUBO), "FillEvaluatedPropsUBO"},
    UniformBlockInfo{5, true, false, sizeof(FillInterpolateUBO), "FillInterpolateUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Float4, 1, "a_outline_color"},
    AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_opacity"},
};
const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{3, true, false, sizeof(FillOutlineDrawableUBO), "FillOutlineDrawableUBO"},
    UniformBlockInfo{4, true, true, sizeof(FillOutlineEvaluatedPropsUBO), "FillOutlineEvaluatedPropsUBO"},
    UniformBlockInfo{5, true, false, sizeof(FillOutlineInterpolateUBO), "FillOutlineInterpolateUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, 1, "a_pattern_from"},
    AttributeInfo{2, gfx::AttributeDataType::UShort4, 1, "a_pattern_to"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_opacity"},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{4, true, true, sizeof(FillPatternDrawableUBO), "FillPatternDrawableUBO"},
    UniformBlockInfo{5, true, true, sizeof(FillPatternTilePropsUBO), "FillPatternTilePropsUBO"},
    UniformBlockInfo{6, true, true, sizeof(FillPatternEvaluatedPropsUBO), "FillPatternEvaluatedPropsUBO"},
    UniformBlockInfo{7, true, false, sizeof(FillPatternInterpolateUBO), "FillPatternInterpolateUBO"},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_image"},
};

const std::array<AttributeInfo, 4>
    ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, 1, "a_pattern_from"},
        AttributeInfo{2, gfx::AttributeDataType::UShort4, 1, "a_pattern_to"},
        AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_opacity"},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::FillOutlinePatternShader,
                                                   gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{4, true, true, sizeof(FillOutlinePatternDrawableUBO), "FillOutlinePatternDrawableUBO"},
    UniformBlockInfo{5, true, true, sizeof(FillOutlinePatternTilePropsUBO), "FillOutlinePatternTilePropsUBO"},
    UniformBlockInfo{6, true, true, sizeof(FillOutlinePatternEvaluatedPropsUBO), "FillOutlinePatternEvaluatedPropsUBO"},
    UniformBlockInfo{7, true, false, sizeof(FillOutlinePatternInterpolateUBO), "FillOutlinePatternInterpolateUBO"},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::textures =
    {
        TextureInfo{0, "u_image"},
};

} // namespace shaders
} // namespace mbgl
