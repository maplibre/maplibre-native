#include <mbgl/shaders/mtl/heatmap.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using HeatmapShaderSource = ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> HeatmapShaderSource::attributes = {
    AttributeInfo{heatmapUBOCount + 0, gfx::AttributeDataType::Short2, idHeatmapPosVertexAttribute},
    AttributeInfo{heatmapUBOCount + 1, gfx::AttributeDataType::Float2, idHeatmapWeightVertexAttribute},
    AttributeInfo{heatmapUBOCount + 2, gfx::AttributeDataType::Float2, idHeatmapRadiusVertexAttribute},
};
const std::array<TextureInfo, 0> HeatmapShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
