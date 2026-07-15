#include <mbgl/shaders/mtl/terrain_depth.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using TerrainDepthShaderSource = ShaderSource<BuiltIn::TerrainDepthShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> TerrainDepthShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, terrainUBOCount + 0, idTerrainPosVertexAttribute},
};
const std::array<TextureInfo, 1> TerrainDepthShaderSource::textures = {
    TextureInfo{0, idTerrainDEMTexture},
};

} // namespace shaders
} // namespace mbgl
