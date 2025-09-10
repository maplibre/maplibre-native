#include <mbgl/shaders/webgpu/symbol.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

// Symbol Icon
using SymbolIconShaderSource = ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 4> SymbolIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idSymbolDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
};

const std::array<TextureInfo, 1> SymbolIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

// Symbol SDF
using SymbolSDFShaderSource = ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 7> SymbolSDFShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idSymbolDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idSymbolOpacityVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
};

const std::array<TextureInfo, 1> SymbolSDFShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

// Symbol Text and Icon
using SymbolTextAndIconShaderSource = ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 7> SymbolTextAndIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idSymbolDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idSymbolOpacityVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
};

const std::array<TextureInfo, 2> SymbolTextAndIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{1, idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl