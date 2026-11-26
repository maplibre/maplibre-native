#include <mln/shaders/mtl/color_relief.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

using ColorReliefShaderSource = ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> ColorReliefShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, colorReliefUBOCount + 0, idColorReliefPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, colorReliefUBOCount + 0, idColorReliefTexturePosVertexAttribute},
};
const std::array<TextureInfo, 3> ColorReliefShaderSource::textures = {
    TextureInfo{0, idColorReliefImageTexture},
    TextureInfo{1, idColorReliefElevationStopsTexture},
    TextureInfo{2, idColorReliefColorStopsTexture},
};

} // namespace shaders
} // namespace mln
