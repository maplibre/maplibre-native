#include <mln/shaders/webgpu/custom_geometry.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/custom_geometry_ubo.hpp>

namespace mln {
namespace shaders {

using CustomGeometryShaderSource = ShaderSource<BuiltIn::CustomGeometryShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> CustomGeometryShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Float3, idCustomGeometryPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idCustomGeometryTexVertexAttribute},
};
const std::array<TextureInfo, 1> CustomGeometryShaderSource::textures = {
    TextureInfo{0, idCustomGeometryTexture},
};

} // namespace shaders
} // namespace mln
