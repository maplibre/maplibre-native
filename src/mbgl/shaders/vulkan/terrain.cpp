#include <mbgl/shaders/vulkan/terrain.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/terrain_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using TerrainShaderSource = ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> TerrainShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, idTerrainPosVertexAttribute},
};
const std::array<TextureInfo, 2> TerrainShaderSource::textures = {
    TextureInfo{0, idTerrainDEMTexture},
    TextureInfo{1, idTerrainMapTexture},
};

} // namespace shaders
} // namespace mbgl
