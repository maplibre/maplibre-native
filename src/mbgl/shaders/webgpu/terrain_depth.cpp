#include <mbgl/shaders/webgpu/terrain_depth.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/terrain_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using TerrainDepthShaderSource = ShaderSource<BuiltIn::TerrainDepthShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> TerrainDepthShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Short2, idTerrainPosVertexAttribute},
};
const std::array<TextureInfo, 1> TerrainDepthShaderSource::textures = {
    TextureInfo{0, idTerrainDEMTexture},
};

} // namespace shaders
} // namespace mbgl
