#include <mln/shaders/mtl/globe_depth.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

using GlobeDepthShaderSource = ShaderSource<BuiltIn::GlobeDepthShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> GlobeDepthShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, globeDepthUBOCount + 0, idGlobeDepthPosVertexAttribute},
};
const std::array<TextureInfo, 0> GlobeDepthShaderSource::textures = {};

} // namespace shaders
} // namespace mln
