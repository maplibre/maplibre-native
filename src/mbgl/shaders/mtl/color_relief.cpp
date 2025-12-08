#include <mbgl/shaders/mtl/color_relief.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using ColorReliefShaderSource = ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::Metal>;

// Note: Attribute indices must match the Metal shader prelude enum values.
// In Metal, colorReliefUBOCount = 5 (based on common.hpp base values), so attributes are at 5 and 6.
// This differs from shader_defines.hpp where colorReliefUBOCount = 4.
const std::array<AttributeInfo, 2> ColorReliefShaderSource::attributes = {
    AttributeInfo{5, gfx::AttributeDataType::Short2, idColorReliefPosVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Short2, idColorReliefTexturePosVertexAttribute},
};

const std::array<TextureInfo, 3> ColorReliefShaderSource::textures = {
    TextureInfo{0, idColorReliefImageTexture},
    TextureInfo{1, idColorReliefElevationStopsTexture},
    TextureInfo{2, idColorReliefColorStopsTexture},
};

} // namespace shaders
} // namespace mbgl
