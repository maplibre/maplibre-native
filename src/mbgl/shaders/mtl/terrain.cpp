#include <mbgl/shaders/mtl/terrain.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using TerrainShaderSource = ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> TerrainShaderSource::attributes = {
    AttributeInfo{terrainUBOCount + 0, gfx::AttributeDataType::Short2, idTerrainPosVertexAttribute},
    AttributeInfo{terrainUBOCount + 1, gfx::AttributeDataType::Short2, idTerrainTexturePosVertexAttribute},
};
const std::array<TextureInfo, 2> TerrainShaderSource::textures = {
    TextureInfo{0, idTerrainDEMTexture},
    TextureInfo{1, idTerrainMapTexture},
};

} // namespace shaders
} // namespace mbgl
