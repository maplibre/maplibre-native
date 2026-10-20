#include <mln/shaders/webgpu/terrain.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/terrain_layer_ubo.hpp>

namespace mln {
namespace shaders {

using TerrainShaderSource = ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> TerrainShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Short4, idTerrainPosVertexAttribute},
};
const std::array<TextureInfo, 2> TerrainShaderSource::textures = {
    TextureInfo{0, idTerrainDEMTexture},
    TextureInfo{1, idTerrainMapTexture},
};

} // namespace shaders
} // namespace mln
