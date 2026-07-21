#include <mbgl/shaders/vulkan/terrain_depth.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/terrain_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using TerrainDepthShaderSource = ShaderSource<BuiltIn::TerrainDepthShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> TerrainDepthShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, idTerrainPosVertexAttribute},
};
const std::array<TextureInfo, 1> TerrainDepthShaderSource::textures = {
    TextureInfo{0, idTerrainDEMTexture},
};

} // namespace shaders
} // namespace mbgl
