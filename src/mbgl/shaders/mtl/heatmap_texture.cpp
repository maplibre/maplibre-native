#include <mbgl/shaders/mtl/heatmap_texture.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal>::uniforms =
    {
        UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, true, sizeof(HeatmapTexturePropsUBO), idHeatmapTexturePropsUBO},
};
const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal>::attributes =
    {
        AttributeInfo{heatmapTextureUBOCount + 0, gfx::AttributeDataType::Short2, idHeatmapPosVertexAttribute},
};
const std::array<TextureInfo, 2> ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idHeatmapImageTexture},
    TextureInfo{1, idHeatmapColorRampTexture},
};

} // namespace shaders
} // namespace mbgl
