#include <mbgl/shaders/vulkan/heatmap.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>
#include <mbgl/shaders/heatmap_texture_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(HeatmapDrawableUBO), idHeatmapDrawableUBO},
    UniformBlockInfo{true, false, sizeof(HeatmapInterpolateUBO), idHeatmapInterpolateUBO},
    UniformBlockInfo{true, true, sizeof(HeatmapEvaluatedPropsUBO), idHeatmapEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idHeatmapPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idHeatmapWeightVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idHeatmapRadiusVertexAttribute},
};

const std::array<UniformBlockInfo, 2>
    ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, true, sizeof(HeatmapTexturePropsUBO), idHeatmapTexturePropsUBO},
};
const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Vulkan>::attributes =
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idHeatmapPosVertexAttribute},
};
const std::array<TextureInfo, 2> ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idHeatmapImageTexture},
    TextureInfo{1, idHeatmapColorRampTexture},
};

} // namespace shaders
} // namespace mbgl
