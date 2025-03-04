#include <mbgl/shaders/vulkan/custom_geometry.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/custom_geometry_ubo.hpp>

namespace mbgl {
namespace shaders {

using CustomGeometryShaderSource = ShaderSource<BuiltIn::CustomGeometryShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 2> CustomGeometryShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float3, idCustomGeometryPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, idCustomGeometryTexVertexAttribute},
};

const std::array<TextureInfo, 1> CustomGeometryShaderSource::textures = {
    TextureInfo{0, idCustomGeometryTexture},
};

} // namespace shaders
} // namespace mbgl
