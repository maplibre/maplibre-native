#include <mbgl/shaders/mtl/custom_geometry.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using CustomGeometryShaderSource = ShaderSource<BuiltIn::CustomGeometryShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> CustomGeometryShaderSource::attributes = {
    AttributeInfo{customGeometryUBOCount + 0, gfx::AttributeDataType::Float3, idCustomGeometryPosVertexAttribute},
    AttributeInfo{customGeometryUBOCount + 1, gfx::AttributeDataType::Float2, idCustomGeometryTexVertexAttribute},
};

const std::array<TextureInfo, 1> CustomGeometryShaderSource::textures = {TextureInfo{0, idCustomGeometryTexture}};

} // namespace shaders
} // namespace mbgl
