#include <mbgl/shaders/mtl/heatmap.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, false, sizeof(HeatmapDrawableUBO), idHeatmapDrawableUBO},
    UniformBlockInfo{true, true, sizeof(HeatmapEvaluatedPropsUBO), idHeatmapEvaluatedPropsUBO},
    UniformBlockInfo{true, false, sizeof(HeatmapInterpolateUBO), idHeatmapInterpolateUBO},
};
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{heatmapUBOCount + 0, gfx::AttributeDataType::Short2, idHeatmapPosVertexAttribute},
    AttributeInfo{heatmapUBOCount + 1, gfx::AttributeDataType::Float2, idHeatmapWeightVertexAttribute},
    AttributeInfo{heatmapUBOCount + 2, gfx::AttributeDataType::Float2, idHeatmapRadiusVertexAttribute},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
