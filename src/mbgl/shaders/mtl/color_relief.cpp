#include <mbgl/shaders/mtl/color_relief.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
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
} // namespace mbgl
