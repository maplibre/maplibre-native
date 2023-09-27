#include <mbgl/shaders/mtl/heatmap_texture.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo,1> ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
};
const std::array<UniformBlockInfo,1> ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{1, true, true, sizeof(HeatmapTextureDrawableUBO), "HeatmapTextureDrawableUBO"},
};
const std::array<TextureInfo,2> ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_image"},
    TextureInfo{1, "u_color_ramp"},
};

} // namespace shaders
} // namespace mbgl
