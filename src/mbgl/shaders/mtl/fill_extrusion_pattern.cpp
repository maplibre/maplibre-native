#include <mbgl/shaders/mtl/fill_extrusion_pattern.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 6>
    ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Short4, "a_normal_ed"},
        AttributeInfo{2, gfx::AttributeDataType::Float, "a_base"},
        AttributeInfo{3, gfx::AttributeDataType::Float, "a_height"},
        AttributeInfo{4, gfx::AttributeDataType::UShort4, "a_pattern_from"},
        AttributeInfo{5, gfx::AttributeDataType::UShort4, "a_pattern_to"},
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
