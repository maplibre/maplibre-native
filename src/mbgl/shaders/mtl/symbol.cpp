#include <mbgl/shaders/mtl/symbol.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Symbol icon

using SymbolIconShaderSource = ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> SymbolIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, symbolUBOCount + 0, idSymbolPosVertexAttribute},

};
const std::array<AttributeInfo, 10> SymbolIconShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::UShort, symbolUBOCount + 1, idSymbolInstanceAttribute},
    
    AttributeInfo{2, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolPosScaleAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolOffsetTlTrAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolOffsetBlBrAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, symbolUBOCount + 2, idSymbolTextureRectAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolPixelOffsetAttribute},
    AttributeInfo{7, gfx::AttributeDataType::UShort4, symbolUBOCount + 2, idSymbolSizeSdfAttribute},

    // Dynamic
    AttributeInfo{8, gfx::AttributeDataType::Float3, symbolUBOCount + 3, idSymbolProjectedPosVertexAttribute},

    // Opacity
    AttributeInfo{9, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolFadeOpacityVertexAttribute},

    // Data driven
    AttributeInfo{10, gfx::AttributeDataType::Float2, symbolUBOCount + 5, idSymbolOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> SymbolIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol sdf

using SymbolSDFShaderSource = ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> SymbolSDFShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, symbolUBOCount + 0, idSymbolPosVertexAttribute},

};
const std::array<AttributeInfo, 14> SymbolSDFShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::UShort, symbolUBOCount + 1, idSymbolInstanceAttribute},
    
    AttributeInfo{2, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolPosScaleAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolOffsetTlTrAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolOffsetBlBrAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, symbolUBOCount + 2, idSymbolTextureRectAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolPixelOffsetAttribute},
    AttributeInfo{7, gfx::AttributeDataType::UShort2, symbolUBOCount + 2, idSymbolSizeSdfAttribute},

    // Dynamic
    AttributeInfo{8, gfx::AttributeDataType::Float3, symbolUBOCount + 3, idSymbolProjectedPosVertexAttribute},

    // Opacity
    AttributeInfo{9, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolFadeOpacityVertexAttribute},

    // Data driven
    AttributeInfo{10, gfx::AttributeDataType::Float2, symbolUBOCount + 5, idSymbolOpacityVertexAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float4, symbolUBOCount + 5, idSymbolColorVertexAttribute},
    AttributeInfo{12, gfx::AttributeDataType::Float4, symbolUBOCount + 5, idSymbolHaloColorVertexAttribute},
    AttributeInfo{13, gfx::AttributeDataType::Float2, symbolUBOCount + 5, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{14, gfx::AttributeDataType::Float2, symbolUBOCount + 5, idSymbolHaloBlurVertexAttribute},
};
const std::array<TextureInfo, 1> SymbolSDFShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
};

//
// Symbol icon and text

using SymbolTextAndIconShaderSource = ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> SymbolTextAndIconShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, symbolUBOCount + 0, idSymbolPosVertexAttribute},

};
const std::array<AttributeInfo, 14> SymbolTextAndIconShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::UShort, symbolUBOCount + 1, idSymbolInstanceAttribute},
    
    AttributeInfo{2, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolPosScaleAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolOffsetTlTrAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolOffsetBlBrAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, symbolUBOCount + 2, idSymbolTextureRectAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Short4, symbolUBOCount + 2, idSymbolPixelOffsetAttribute},
    AttributeInfo{7, gfx::AttributeDataType::UShort2, symbolUBOCount + 2, idSymbolSizeSdfAttribute},

    // Dynamic
    AttributeInfo{8, gfx::AttributeDataType::Float3, symbolUBOCount + 3, idSymbolProjectedPosVertexAttribute},

    // Opacity
    AttributeInfo{9, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolFadeOpacityVertexAttribute},

    // Data driven
    AttributeInfo{10, gfx::AttributeDataType::Float2, symbolUBOCount + 5, idSymbolOpacityVertexAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float4, symbolUBOCount + 5, idSymbolColorVertexAttribute},
    AttributeInfo{12, gfx::AttributeDataType::Float4, symbolUBOCount + 5, idSymbolHaloColorVertexAttribute},
    AttributeInfo{13, gfx::AttributeDataType::Float2, symbolUBOCount + 5, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{14, gfx::AttributeDataType::Float2, symbolUBOCount + 5, idSymbolHaloBlurVertexAttribute},
};
const std::array<TextureInfo, 2> SymbolTextAndIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{1, idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
