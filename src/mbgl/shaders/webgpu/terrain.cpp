#include <mbgl/shaders/webgpu/terrain.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/terrain_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using TerrainShaderSource = ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> TerrainShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Short2, idTerrainPosVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Short2, idTerrainTexturePosVertexAttribute},
};
const std::array<TextureInfo, 2> TerrainShaderSource::textures = {
    TextureInfo{0, idTerrainDEMTexture},
    TextureInfo{1, idTerrainMapTexture},
};

} // namespace shaders
} // namespace mbgl
