#include <mbgl/shaders/mtl/fill.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Float4, "a_color"},
    AttributeInfo{2, gfx::AttributeDataType::Float2, "a_opacity"},
};
const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{3, true, false, sizeof(FillDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{4, true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
    UniformBlockInfo{5, true, false, sizeof(FillInterpolateUBO), idFillInterpolateUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Float4, "a_outline_color"},
    AttributeInfo{2, gfx::AttributeDataType::Float2, "a_opacity"},
};
const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{3, true, false, sizeof(FillOutlineDrawableUBO), idFillOutlineDrawableUBO},
    UniformBlockInfo{4, true, true, sizeof(FillOutlineEvaluatedPropsUBO), idFillOutlineEvaluatedPropsUBO},
    UniformBlockInfo{5, true, false, sizeof(FillOutlineInterpolateUBO), idFillOutlineInterpolateUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, "a_pattern_from"},
    AttributeInfo{2, gfx::AttributeDataType::UShort4, "a_pattern_to"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, "a_opacity"},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{4, true, true, sizeof(FillPatternDrawableUBO), idFillPatternDrawableUBO},
    UniformBlockInfo{5, true, true, sizeof(FillPatternTilePropsUBO), idFillPatternTilePropsUBO},
    UniformBlockInfo{6, true, true, sizeof(FillPatternEvaluatedPropsUBO), idFillPatternEvaluatedPropsUBO},
    UniformBlockInfo{7, true, false, sizeof(FillPatternInterpolateUBO), idFillPatternInterpolateUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_image"},
};

const std::array<AttributeInfo, 4>
    ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, "a_pattern_from"},
        AttributeInfo{2, gfx::AttributeDataType::UShort4, "a_pattern_to"},
        AttributeInfo{3, gfx::AttributeDataType::Float2, "a_opacity"},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::FillOutlinePatternShader,
                                                   gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{4, true, true, sizeof(FillOutlinePatternDrawableUBO), idFillOutlinePatternDrawableUBO},
    UniformBlockInfo{5, true, true, sizeof(FillOutlinePatternTilePropsUBO), idFillOutlinePatternTilePropsUBO},
    UniformBlockInfo{6, true, true, sizeof(FillOutlinePatternEvaluatedPropsUBO), idFillOutlinePatternEvaluatedPropsUBO},
    UniformBlockInfo{7, true, false, sizeof(FillOutlinePatternInterpolateUBO), idFillOutlinePatternInterpolateUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::textures =
    {
        TextureInfo{0, "u_image"},
};

} // namespace shaders
} // namespace mbgl
