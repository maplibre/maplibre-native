#include <mln/shaders/mtl/sky.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

using SkyShaderSource = ShaderSource<BuiltIn::SkyShader, gfx::Backend::Type::Metal>;
const std::array<AttributeInfo, 1> SkyShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, skyLayerUBOCount + 0, idSkyPosVertexAttribute},
};
const std::array<TextureInfo, 0> SkyShaderSource::textures = {};

using AtmosphereShaderSource = ShaderSource<BuiltIn::AtmosphereShader, gfx::Backend::Type::Metal>;
const std::array<AttributeInfo, 1> AtmosphereShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, atmosphereLayerUBOCount + 0, idAtmospherePosVertexAttribute},
};
const std::array<TextureInfo, 0> AtmosphereShaderSource::textures = {};

} // namespace shaders
} // namespace mln
