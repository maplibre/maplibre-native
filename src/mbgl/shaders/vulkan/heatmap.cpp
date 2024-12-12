#include <mbgl/shaders/vulkan/heatmap.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using HeatmapShaderSource = ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Vulkan>;

const std::array<UniformBlockInfo, 2> HeatmapShaderSource::uniforms = {
    UniformBlockInfo{true, false, sizeof(HeatmapDrawableUBO), idHeatmapDrawableUBO},
    UniformBlockInfo{true, true, sizeof(HeatmapEvaluatedPropsUBO), idHeatmapEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 3> HeatmapShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idHeatmapPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idHeatmapWeightVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idHeatmapRadiusVertexAttribute},
};
const std::array<TextureInfo, 0> HeatmapShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
