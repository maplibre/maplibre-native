#include <mbgl/shaders/webgpu/custom_geometry.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/custom_geometry_ubo.hpp>

namespace mbgl {
namespace shaders {

using CustomGeometryShaderSource = ShaderSource<BuiltIn::CustomGeometryShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> CustomGeometryShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idCustomGeometryPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idCustomGeometryNormalVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idCustomGeometryTexCoordVertexAttribute},
};
const std::array<TextureInfo, 0> CustomGeometryShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl