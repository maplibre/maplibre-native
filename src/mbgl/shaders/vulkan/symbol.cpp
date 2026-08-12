#include <mbgl/shaders/vulkan/symbol.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Symbol icon

using SymbolIconShaderSource = ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 6> SymbolIconShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, idSymbolDataAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short4, idSymbolPixelOffsetAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float3, idSymbolProjectedPosAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, idSymbolFadeOpacityAttribute},

    // sometimes uniforms
    AttributeInfo{5, gfx::AttributeDataType::Float, idSymbolOpacityAttribute},
};
const std::array<TextureInfo, 1> SymbolIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol sdf

using SymbolSDFShaderSource = ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 10> SymbolSDFShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, idSymbolDataAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short4, idSymbolPixelOffsetAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float3, idSymbolProjectedPosAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, idSymbolFadeOpacityAttribute},

    // sometimes uniforms
    AttributeInfo{5, gfx::AttributeDataType::Float4, idSymbolColorAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float4, idSymbolHaloColorAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float, idSymbolOpacityAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float, idSymbolHaloWidthAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float, idSymbolHaloBlurAttribute},
};
const std::array<TextureInfo, 1> SymbolSDFShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol icon and text

using SymbolTextAndIconShaderSource = ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 9> SymbolTextAndIconShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, idSymbolDataAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float3, idSymbolProjectedPosAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float, idSymbolFadeOpacityAttribute},

    // sometimes uniforms
    AttributeInfo{4, gfx::AttributeDataType::Float4, idSymbolColorAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float4, idSymbolHaloColorAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float, idSymbolOpacityAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float, idSymbolHaloWidthAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float, idSymbolHaloBlurAttribute},
};
const std::array<TextureInfo, 2> SymbolTextAndIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{1, idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
