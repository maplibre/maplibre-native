#include <mln/shaders/mtl/custom_geometry.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

using CustomGeometryShaderSource = ShaderSource<BuiltIn::CustomGeometryShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> CustomGeometryShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float3, customGeometryUBOCount + 0, idCustomGeometryPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float2, customGeometryUBOCount + 0, idCustomGeometryTexVertexAttribute},
};

const std::array<TextureInfo, 1> CustomGeometryShaderSource::textures = {TextureInfo{0, idCustomGeometryTexture}};

} // namespace shaders
} // namespace mln
