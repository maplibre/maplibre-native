#include <mbgl/shaders/webgpu/symbol.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Symbol icon

using SymbolIconShaderSource = ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 4> SymbolIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idSymbolDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> SymbolIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture}
};

//
// Symbol SDF

using SymbolSDFShaderSource = ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 4> SymbolSDFShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idSymbolDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> SymbolSDFShaderSource::textures = {
    TextureInfo{0, idSymbolGlyphTexture}
};

} // namespace shaders
} // namespace mbgl