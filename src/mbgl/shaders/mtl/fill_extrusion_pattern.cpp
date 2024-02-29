#include <mbgl/shaders/mtl/fill_extrusion_pattern.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 6>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
        AttributeInfo{1, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
        AttributeInfo{2, gfx::AttributeDataType::Float, idFillExtrusionBaseVertexAttribute},
        AttributeInfo{3, gfx::AttributeDataType::Float, idFillExtrusionHeightVertexAttribute},
        AttributeInfo{4, gfx::AttributeDataType::UShort4, idFillExtrusionPatternFromVertexAttribute},
        AttributeInfo{5, gfx::AttributeDataType::UShort4, idFillExtrusionPatternToVertexAttribute},
};
const std::array<UniformBlockInfo, 4>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{6, true, true, sizeof(FillExtrusionDrawableUBO), idFillExtrusionDrawableUBO},
        UniformBlockInfo{7, true, true, sizeof(FillExtrusionDrawablePropsUBO), idFillExtrusionDrawablePropsUBO},
        UniformBlockInfo{8, true, true, sizeof(FillExtrusionDrawableTilePropsUBO), idFillExtrusionDrawableTilePropsUBO},
        UniformBlockInfo{9, true, false, sizeof(FillExtrusionInterpolateUBO), idFillExtrusionInterpolateUBO},
};
const std::array<TextureInfo, 1>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::textures = {
        TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mbgl
