#include <mln/shaders/webgpu/symbol.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/symbol_layer_ubo.hpp>

namespace mln {
namespace shaders {

// Symbol Icon
using SymbolIconShaderSource = ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 6> SymbolIconShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short4, idSymbolPosOffsetAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idSymbolDataAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short4, idSymbolPixelOffsetAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float3, idSymbolProjectedPosAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float, idSymbolFadeOpacityAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float, idSymbolOpacityAttribute},
};

const std::array<TextureInfo, 3> SymbolIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{2, idSymbolDEMTexture},
    TextureInfo{4, idSymbolDepthTexture},
};

// Symbol SDF
using SymbolSDFShaderSource = ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 10> SymbolSDFShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short4, idSymbolPosOffsetAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idSymbolDataAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short4, idSymbolPixelOffsetAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float3, idSymbolProjectedPosAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float, idSymbolFadeOpacityAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float4, idSymbolColorAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float4, idSymbolHaloColorAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float, idSymbolOpacityAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float, idSymbolHaloWidthAttribute},
    AttributeInfo{12, gfx::AttributeDataType::Float, idSymbolHaloBlurAttribute},
};

const std::array<TextureInfo, 3> SymbolSDFShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{2, idSymbolDEMTexture},
    TextureInfo{4, idSymbolDepthTexture},
};

// Symbol Text and Icon
using SymbolTextAndIconShaderSource = ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 9> SymbolTextAndIconShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short4, idSymbolPosOffsetAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idSymbolDataAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float3, idSymbolProjectedPosAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float, idSymbolFadeOpacityAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float4, idSymbolColorAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float4, idSymbolHaloColorAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float, idSymbolOpacityAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float, idSymbolHaloWidthAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float, idSymbolHaloBlurAttribute},
};

const std::array<TextureInfo, 4> SymbolTextAndIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{2, idSymbolImageIconTexture},
    TextureInfo{4, idSymbolDEMTexture},
    TextureInfo{6, idSymbolDepthTexture},
};

} // namespace shaders
} // namespace mln
