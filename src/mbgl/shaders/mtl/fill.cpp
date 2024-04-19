#include <mbgl/shaders/mtl/fill.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, false, sizeof(FillDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{true, false, sizeof(FillInterpolateUBO), idFillInterpolateUBO},
    UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::Float4, idFillColorVertexAttribute},
    AttributeInfo{fillUBOCount + 2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, false, sizeof(FillOutlineDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{true, false, sizeof(FillOutlineInterpolateUBO), idFillInterpolateUBO},
    UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::Float4, idFillOutlineColorVertexAttribute},
    AttributeInfo{fillUBOCount + 2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>::textures = {};

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, true, sizeof(FillPatternDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{true, true, sizeof(FillPatternTilePropsUBO), idFillTilePropsUBO},
    UniformBlockInfo{true, false, sizeof(FillPatternInterpolateUBO), idFillInterpolateUBO},
    UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{fillUBOCount + 2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{fillUBOCount + 3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idFillImageTexture},
};

const std::array<UniformBlockInfo, 4>
    ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{true, true, sizeof(FillOutlinePatternDrawableUBO), idFillDrawableUBO},
        UniformBlockInfo{true, true, sizeof(FillOutlinePatternTilePropsUBO), idFillTilePropsUBO},
        UniformBlockInfo{true, false, sizeof(FillOutlinePatternInterpolateUBO), idFillInterpolateUBO},
        UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 4>
    ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::attributes = {
        AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
        AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
        AttributeInfo{fillUBOCount + 2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
        AttributeInfo{fillUBOCount + 3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>::textures =
    {
        TextureInfo{0, idFillImageTexture},
};

const std::array<UniformBlockInfo, 2>
    ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{true, false, sizeof(FillOutlineTriangulatedDrawableUBO), idFillDrawableUBO},
        UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 2>
    ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Metal>::attributes = {
        AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
        AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 0>
    ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
