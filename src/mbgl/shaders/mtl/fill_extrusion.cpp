#include <mbgl/shaders/mtl/fill_extrusion.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>;

const std::array<UniformBlockInfo, 2> FillExtrusionShaderSource::uniforms = {
        UniformBlockInfo{true, false, sizeof(FillExtrusionDrawableUBO), idFillExtrusionDrawableUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionPropsUBO), idFillExtrusionPropsUBO},
};
const std::array<AttributeInfo, 5> FillExtrusionShaderSource::attributes = {
    AttributeInfo{fillExtrusionUBOCount + 0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 1, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 2, gfx::AttributeDataType::Float4, idFillExtrusionColorVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 3, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 4, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShaderSource::textures = {};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>;

const std::array<UniformBlockInfo, 4> FillExtrusionPatternShaderSource::uniforms = {
        UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionDrawableUBO), idFillExtrusionDrawableUBO},
        UniformBlockInfo{true, true, sizeof(FillExtrusionTilePropsUBO), idFillExtrusionTilePropsUBO},
        UniformBlockInfo{true, true, sizeof(FillExtrusionPropsUBO), idFillExtrusionPropsUBO},
};
const std::array<AttributeInfo, 6> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{fillExtrusionUBOCount + 0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 1, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 2, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 3, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 4, gfx::AttributeDataType::UShort4, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 5, gfx::AttributeDataType::UShort4, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternShaderSource::textures = {
        TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mbgl
