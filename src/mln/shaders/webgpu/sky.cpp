#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/webgpu/sky.hpp>

namespace mln {
namespace shaders {

using SkyShaderSource = ShaderSource<BuiltIn::SkyShader, gfx::Backend::Type::WebGPU>;
const std::array<AttributeInfo, 1> SkyShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idSkyPosVertexAttribute},
};
const std::array<TextureInfo, 0> SkyShaderSource::textures = {};

using AtmosphereShaderSource = ShaderSource<BuiltIn::AtmosphereShader, gfx::Backend::Type::WebGPU>;
const std::array<AttributeInfo, 1> AtmosphereShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idAtmospherePosVertexAttribute},
};
const std::array<TextureInfo, 0> AtmosphereShaderSource::textures = {};

} // namespace shaders
} // namespace mln
