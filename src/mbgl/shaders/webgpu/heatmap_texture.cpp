#include <mbgl/shaders/webgpu/heatmap_texture.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/heatmap_texture_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using HeatmapTextureShaderSource = ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> HeatmapTextureShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Short2, idHeatmapPosVertexAttribute},
};
const std::array<TextureInfo, 2> HeatmapTextureShaderSource::textures = {TextureInfo{0, idHeatmapImageTexture},
                                                                         TextureInfo{1, idHeatmapColorRampTexture}};

} // namespace shaders
} // namespace mbgl
