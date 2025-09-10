#include <mbgl/shaders/webgpu/heatmap.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using HeatmapShaderSource = ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> HeatmapShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idHeatmapPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float, idHeatmapWeightVertexAttribute},
};
const std::array<TextureInfo, 0> HeatmapShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl