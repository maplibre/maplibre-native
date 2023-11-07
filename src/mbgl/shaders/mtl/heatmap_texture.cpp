// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/heatmap_texture.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal>::reflectionData = {
    "HeatmapTextureShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
    },
    {
        UniformBlockInfo{1, true, true, sizeof(HeatmapTextureDrawableUBO), "HeatmapTextureDrawableUBO"},
    },
    {
        TextureInfo{0, "u_image"},
        TextureInfo{1, "u_color_ramp"},
    },
    {
        BuiltIn::Prelude,
    },
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
