#include <mbgl/shaders/mtl/fill.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Float4, 1, "a_color"},
    AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_opacity"},

    // This shader doesn't use it, but we need this so that the layer can assign the same
    // attributes to all the drawables.  If/When `VertexAttributeArray::resolve` allows it,
    // this can be removed.
    AttributeInfo{8, gfx::AttributeDataType::Float2, 1, "a_outline_color"},
};
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{3, true, false, sizeof(FillDrawableUBO), "FillDrawableUBO"},
    UniformBlockInfo{4, true, true, sizeof(FillEvaluatedPropsUBO), "FillEvaluatedPropsUBO"},
    UniformBlockInfo{5, true, false, sizeof(FillInterpolateUBO), "FillInterpolateUBO"},
    UniformBlockInfo{6, true, true, sizeof(FillPermutationUBO), "FillPermutationUBO"},
    UniformBlockInfo{7, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Float4, 1, "a_outline_color"},
    AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_opacity"},

    // See `a_outline_color` in FillShader
    AttributeInfo{8, gfx::AttributeDataType::Float2, 1, "a_color"},
};
const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{3, true, false, sizeof(FillOutlineDrawableUBO), "FillOutlineDrawableUBO"},
    UniformBlockInfo{4, true, true, sizeof(FillOutlineEvaluatedPropsUBO), "FillOutlineEvaluatedPropsUBO"},
    UniformBlockInfo{5, true, false, sizeof(FillOutlineInterpolateUBO), "FillOutlineInterpolateUBO"},
    UniformBlockInfo{6, true, true, sizeof(FillOutlinePermutationUBO), "FillOutlinePermutationUBO"},
    UniformBlockInfo{7, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, 1, "a_pattern_from"},
    AttributeInfo{2, gfx::AttributeDataType::UShort4, 1, "a_pattern_to"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_opacity"},
};
const std::array<UniformBlockInfo, 6> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{4, true, true, sizeof(FillPatternDrawableUBO), "FillPatternDrawableUBO"},
    UniformBlockInfo{5, true, false, sizeof(FillPatternTilePropsUBO), "FillPatternTilePropsUBO"},
    UniformBlockInfo{6, true, true, sizeof(FillPatternEvaluatedPropsUBO), "FillPatternEvaluatedPropsUBO"},
    UniformBlockInfo{7, true, false, sizeof(FillPatternInterpolateUBO), "FillPatternInterpolateUBO"},
    UniformBlockInfo{8, true, true, sizeof(FillPatternPermutationUBO), "FillPatternPermutationUBO"},
    UniformBlockInfo{9, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
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
const std::array<UniformBlockInfo, 6> ShaderSource<BuiltIn::FillOutlinePatternShader,
                                                   gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{4, true, true, sizeof(FillOutlinePatternDrawableUBO), "FillOutlinePatternDrawableUBO"},
    UniformBlockInfo{5, true, false, sizeof(FillOutlinePatternTilePropsUBO), "FillOutlinePatternTilePropsUBO"},
    UniformBlockInfo{6, true, true, sizeof(FillOutlinePatternEvaluatedPropsUBO), "FillOutlinePatternEvaluatedPropsUBO"},
    UniformBlockInfo{7, true, false, sizeof(FillOutlinePatternInterpolateUBO), "FillOutlinePatternInterpolateUBO"},
    UniformBlockInfo{8, true, true, sizeof(FillOutlinePatternPermutationUBO), "FillOutlinePatternPermutationUBO"},
    UniformBlockInfo{9, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::textures =
    {
        TextureInfo{0, "u_image"},
};

} // namespace shaders
} // namespace mbgl
