#include <mbgl/shaders/mtl/heatmap_texture.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using HeatmapTextureShaderSource = ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> HeatmapTextureShaderSource::attributes = {
    AttributeInfo{heatmapTextureUBOCount + 0, gfx::AttributeDataType::Short2, idHeatmapPosVertexAttribute},
};
const std::array<TextureInfo, 2> HeatmapTextureShaderSource::textures = {
    TextureInfo{0, idHeatmapImageTexture},
    TextureInfo{1, idHeatmapColorRampTexture},
};

} // namespace shaders
} // namespace mbgl
