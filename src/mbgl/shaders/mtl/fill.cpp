#include <mbgl/shaders/mtl/fill.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{3, true, false, sizeof(FillDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{4, true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
    UniformBlockInfo{5, true, false, sizeof(FillInterpolateUBO), idFillInterpolateUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillOutlineColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{3, true, false, sizeof(FillOutlineDrawableUBO), idFillOutlineDrawableUBO},
    UniformBlockInfo{4, true, true, sizeof(FillOutlineEvaluatedPropsUBO), idFillOutlineEvaluatedPropsUBO},
    UniformBlockInfo{5, true, false, sizeof(FillOutlineInterpolateUBO), idFillOutlineInterpolateUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{4, true, true, sizeof(FillPatternDrawableUBO), idFillPatternDrawableUBO},
    UniformBlockInfo{5, true, true, sizeof(FillPatternTilePropsUBO), idFillPatternTilePropsUBO},
    UniformBlockInfo{6, true, true, sizeof(FillPatternEvaluatedPropsUBO), idFillPatternEvaluatedPropsUBO},
    UniformBlockInfo{7, true, false, sizeof(FillPatternInterpolateUBO), idFillPatternInterpolateUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idFillImageTexture},
};

const std::array<AttributeInfo, 4>
    ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
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
        TextureInfo{0, idFillImageTexture},
};

} // namespace shaders
} // namespace mbgl
