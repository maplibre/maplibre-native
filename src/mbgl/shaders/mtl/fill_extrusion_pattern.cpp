#include <mbgl/shaders/mtl/fill_extrusion_pattern.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 5>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, true, sizeof(FillExtrusionDrawableUBO), idFillExtrusionDrawableUBO},
        UniformBlockInfo{true, true, sizeof(FillExtrusionPropsUBO), idFillExtrusionPropsUBO},
        UniformBlockInfo{true, true, sizeof(FillExtrusionTilePropsUBO), idFillExtrusionTilePropsUBO},
        UniformBlockInfo{true, false, sizeof(FillExtrusionInterpolateUBO), idFillExtrusionInterpolateUBO},
};
const std::array<AttributeInfo, 6> ShaderSource<BuiltIn::FillExtrusionPatternShader,
                                                gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{fillExtrusionUBOCount + 0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 1, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 2, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 3, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{
        fillExtrusionUBOCount + 4, gfx::AttributeDataType::UShort4, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{fillExtrusionUBOCount + 5, gfx::AttributeDataType::UShort4, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::textures = {
        TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mbgl
