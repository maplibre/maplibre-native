#include <mbgl/shaders/mtl/heatmap.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using HeatmapShaderSource = ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> HeatmapShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, heatmapUBOCount + 0, idHeatmapPosVertexAttribute},

    // Data driven
    AttributeInfo{1, gfx::AttributeDataType::Float2, heatmapUBOCount + 1, idHeatmapWeightVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, heatmapUBOCount + 1, idHeatmapRadiusVertexAttribute},
};
const std::array<TextureInfo, 0> HeatmapShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
