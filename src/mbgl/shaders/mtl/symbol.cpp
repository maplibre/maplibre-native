#include <mbgl/shaders/mtl/symbol.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Symbol icon

using SymbolIconShaderSource = ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 6> SymbolIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, symbolUBOCount + 0, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, symbolUBOCount + 0, idSymbolDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short4, symbolUBOCount + 0, idSymbolPixelOffsetVertexAttribute},
    
    // Dynamic
    AttributeInfo{3, gfx::AttributeDataType::Float3, symbolUBOCount + 1, idSymbolProjectedPosVertexAttribute},
    
    // Opacity
    AttributeInfo{4, gfx::AttributeDataType::Float, symbolUBOCount + 2, idSymbolFadeOpacityVertexAttribute},

    // Data driven
    AttributeInfo{5, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> SymbolIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol sdf

using SymbolSDFShaderSource = ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 10> SymbolSDFShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, symbolUBOCount + 0, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, symbolUBOCount + 0, idSymbolDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short4, symbolUBOCount + 0, idSymbolPixelOffsetVertexAttribute},
    
    // Dynamic
    AttributeInfo{3, gfx::AttributeDataType::Float3, symbolUBOCount + 1, idSymbolProjectedPosVertexAttribute},
    
    // Opacity
    AttributeInfo{4, gfx::AttributeDataType::Float, symbolUBOCount + 2, idSymbolFadeOpacityVertexAttribute},

    // Data driven
    AttributeInfo{5, gfx::AttributeDataType::Float4, symbolUBOCount + 3, idSymbolColorVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float4, symbolUBOCount + 3, idSymbolHaloColorVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolOpacityVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{9, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolHaloBlurVertexAttribute},
};
const std::array<TextureInfo, 1> SymbolSDFShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol icon and text

using SymbolTextAndIconShaderSource = ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 9> SymbolTextAndIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short4, symbolUBOCount + 0, idSymbolPosOffsetVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, symbolUBOCount + 0, idSymbolDataVertexAttribute},
    
    // Dynamic
    AttributeInfo{2, gfx::AttributeDataType::Float3, symbolUBOCount + 1, idSymbolProjectedPosVertexAttribute},
    
    // Opacity
    AttributeInfo{3, gfx::AttributeDataType::Float, symbolUBOCount + 2, idSymbolFadeOpacityVertexAttribute},

    // Data driven
    AttributeInfo{4, gfx::AttributeDataType::Float4, symbolUBOCount + 3, idSymbolColorVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float4, symbolUBOCount + 3, idSymbolHaloColorVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolOpacityVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolHaloBlurVertexAttribute},
};
const std::array<TextureInfo, 2> SymbolTextAndIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{1, idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
