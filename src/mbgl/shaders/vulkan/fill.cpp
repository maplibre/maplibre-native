#include <mbgl/shaders/vulkan/fill.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(FillDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{true, false, sizeof(FillInterpolateUBO), idFillInterpolateUBO},
    UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, false, sizeof(FillOutlineDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{true, false, sizeof(FillOutlineInterpolateUBO), idFillInterpolateUBO},
    UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillOutlineColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};

const std::array<UniformBlockInfo, 5> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, true, sizeof(FillPatternDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{true, true, sizeof(FillPatternTilePropsUBO), idFillTilePropsUBO},
    UniformBlockInfo{true, false, sizeof(FillPatternInterpolateUBO), idFillInterpolateUBO},
    UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 4> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idFillImageTexture},
};

const std::array<UniformBlockInfo, 5>
    ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, true, sizeof(FillOutlinePatternDrawableUBO), idFillDrawableUBO},
        UniformBlockInfo{true, true, sizeof(FillOutlinePatternTilePropsUBO), idFillTilePropsUBO},
        UniformBlockInfo{true, false, sizeof(FillOutlinePatternInterpolateUBO), idFillInterpolateUBO},
        UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 4>
    ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Vulkan>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Vulkan>::textures =
    {
        TextureInfo{0, idFillImageTexture},
};

const std::array<UniformBlockInfo, 3>
    ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, false, sizeof(FillOutlineTriangulatedDrawableUBO), idFillDrawableUBO},
        UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 2>
    ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Vulkan>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
};

const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Vulkan>::uniforms =
    {
        UniformBlockInfo{true, false, sizeof(FillExtrusionDrawableUBO), idFillExtrusionDrawableUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionPropsUBO), idFillExtrusionPropsUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionInterpolateUBO), idFillExtrusionInterpolateUBO},
};
const std::array<AttributeInfo, 5> ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Vulkan>::attributes =
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::Float4, idFillExtrusionColorVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::Float2, idFillExtrusionBaseVertexAttribute},
        AttributeInfo{4, gfx::AttributeDataType::Float2, idFillExtrusionHeightVertexAttribute},
};

const std::array<UniformBlockInfo, 5>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, true, sizeof(FillExtrusionDrawableUBO), idFillExtrusionDrawableUBO},
        UniformBlockInfo{true, true, sizeof(FillExtrusionPropsUBO), idFillExtrusionPropsUBO},
        UniformBlockInfo{true, true, sizeof(FillExtrusionTilePropsUBO), idFillExtrusionTilePropsUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionInterpolateUBO), idFillExtrusionInterpolateUBO},
};
const std::array<AttributeInfo, 6>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Vulkan>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::Float2, idFillExtrusionBaseVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::Float2, idFillExtrusionHeightVertexAttribute},
        AttributeInfo{4, gfx::AttributeDataType::UShort4, idFillExtrusionPatternFromVertexAttribute},
        AttributeInfo{5, gfx::AttributeDataType::UShort4, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Vulkan>::textures = {
        TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mbgl
