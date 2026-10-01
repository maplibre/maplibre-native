#include <mln/shaders/webgpu/heatmap.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/heatmap_layer_ubo.hpp>

namespace mln {
namespace shaders {

using HeatmapShaderSource = ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> HeatmapShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idHeatmapPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idHeatmapWeightVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idHeatmapRadiusVertexAttribute},
};
const std::array<TextureInfo, 0> HeatmapShaderSource::textures = {};

} // namespace shaders
} // namespace mln
