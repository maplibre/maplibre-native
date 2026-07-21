#include <mbgl/shaders/mtl/terrain.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using TerrainShaderSource = ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> TerrainShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, terrainUBOCount + 0, idTerrainPosVertexAttribute},
};
const std::array<TextureInfo, 2> TerrainShaderSource::textures = {
    TextureInfo{0, idTerrainDEMTexture},
    TextureInfo{1, idTerrainMapTexture},
};

} // namespace shaders
} // namespace mbgl
