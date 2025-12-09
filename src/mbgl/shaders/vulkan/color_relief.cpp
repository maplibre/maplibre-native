#include <mbgl/shaders/vulkan/color_relief.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using ColorReliefShaderSource = ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 2> ColorReliefShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idColorReliefPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, idColorReliefTexturePosVertexAttribute},
};

const std::array<TextureInfo, 3> ColorReliefShaderSource::textures = {
    TextureInfo{0, idColorReliefImageTexture},
    TextureInfo{1, idColorReliefElevationStopsTexture},
    TextureInfo{2, idColorReliefColorStopsTexture},
};

} // namespace shaders
} // namespace mbgl
