#include <mbgl/shaders/mtl/symbol.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Symbol icon

using SymbolIconShaderSource = ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 6> SymbolIconShaderSource::attributes = {
    // always attributes
    AttributeInfo{symbolUBOCount + 0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{symbolUBOCount + 1, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
    AttributeInfo{symbolUBOCount + 2, gfx::AttributeDataType::Short4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{symbolUBOCount + 3, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{symbolUBOCount + 4, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},

    // sometimes uniforms
    AttributeInfo{symbolUBOCount + 5, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> SymbolIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol sdf

using SymbolSDFShaderSource = ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 10> SymbolSDFShaderSource::attributes = {
    // always attributes
    AttributeInfo{symbolUBOCount + 0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{symbolUBOCount + 1, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
    AttributeInfo{symbolUBOCount + 2, gfx::AttributeDataType::Short4, idSymbolPixelOffsetVertexAttribute},
    AttributeInfo{symbolUBOCount + 3, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{symbolUBOCount + 4, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},

    // sometimes uniforms
    AttributeInfo{symbolUBOCount + 5, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
    AttributeInfo{symbolUBOCount + 6, gfx::AttributeDataType::Float4, idSymbolHaloColorVertexAttribute},
    AttributeInfo{symbolUBOCount + 7, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
    AttributeInfo{symbolUBOCount + 8, gfx::AttributeDataType::Float, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{symbolUBOCount + 9, gfx::AttributeDataType::Float, idSymbolHaloBlurVertexAttribute},
};
const std::array<TextureInfo, 1> SymbolSDFShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol icon and text

using SymbolTextAndIconShaderSource = ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 9> SymbolTextAndIconShaderSource::attributes = {
    // always attributes
    AttributeInfo{symbolUBOCount + 0, gfx::AttributeDataType::Short4, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{symbolUBOCount + 1, gfx::AttributeDataType::UShort4, idSymbolDataVertexAttribute},
    AttributeInfo{symbolUBOCount + 2, gfx::AttributeDataType::Float3, idSymbolProjectedPosVertexAttribute},
    AttributeInfo{symbolUBOCount + 3, gfx::AttributeDataType::Float, idSymbolFadeOpacityVertexAttribute},

    // sometimes uniforms
    AttributeInfo{symbolUBOCount + 4, gfx::AttributeDataType::Float4, idSymbolColorVertexAttribute},
    AttributeInfo{symbolUBOCount + 5, gfx::AttributeDataType::Float4, idSymbolHaloColorVertexAttribute},
    AttributeInfo{symbolUBOCount + 6, gfx::AttributeDataType::Float, idSymbolOpacityVertexAttribute},
    AttributeInfo{symbolUBOCount + 7, gfx::AttributeDataType::Float, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{symbolUBOCount + 8, gfx::AttributeDataType::Float, idSymbolHaloBlurVertexAttribute},
};
const std::array<TextureInfo, 2> SymbolTextAndIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{1, idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
