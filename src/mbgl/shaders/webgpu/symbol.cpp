#include <mbgl/shaders/webgpu/symbol.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

// Symbol Icon
using SymbolIconShaderSource = ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 6> SymbolIconShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
};

const std::array<TextureInfo, 1> SymbolIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

// Symbol SDF
using SymbolSDFShaderSource = ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 10> SymbolSDFShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float4, idSymbolHaloColorVertexAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{12, gfx::AttributeDataType::Float, idSymbolHaloBlurVertexAttribute},
};

const std::array<TextureInfo, 1> SymbolSDFShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

// Symbol Text and Icon
using SymbolTextAndIconShaderSource = ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 9> SymbolTextAndIconShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float4, idSymbolHaloColorVertexAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float, idSymbolHaloBlurVertexAttribute},
};

const std::array<TextureInfo, 2> SymbolTextAndIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{2, idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
