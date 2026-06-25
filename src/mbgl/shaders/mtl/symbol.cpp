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
const std::array<AttributeInfo, 9> SymbolIconShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolPosScaleAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolOffsetTlTrAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolOffsetBlBrAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, symbolUBOCount + 1, idSymbolTextureRectAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolPixelOffsetAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UShort2, symbolUBOCount + 1, idSymbolSizeSdfAttribute},

    // Dynamic
    AttributeInfo{7, gfx::AttributeDataType::Float3, symbolUBOCount + 2, idSymbolProjectedPosVertexAttribute},

    // Opacity
    AttributeInfo{8, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolFadeOpacityVertexAttribute},

    // Data driven
    AttributeInfo{9, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolOpacityVertexAttribute},
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
const std::array<AttributeInfo, 13> SymbolSDFShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolPosScaleAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolOffsetTlTrAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolOffsetBlBrAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, symbolUBOCount + 1, idSymbolTextureRectAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolPixelOffsetAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UShort2, symbolUBOCount + 1, idSymbolSizeSdfAttribute},

    // Dynamic
    AttributeInfo{7, gfx::AttributeDataType::Float3, symbolUBOCount + 2, idSymbolProjectedPosVertexAttribute},

    // Opacity
    AttributeInfo{8, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolFadeOpacityVertexAttribute},

    // Data driven
    AttributeInfo{9, gfx::AttributeDataType::Float4, symbolUBOCount + 4, idSymbolColorVertexAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float4, symbolUBOCount + 4, idSymbolHaloColorVertexAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolOpacityVertexAttribute},
    AttributeInfo{12, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{13, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolHaloBlurVertexAttribute},
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
const std::array<AttributeInfo, 13> SymbolTextAndIconShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolPosScaleAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolOffsetTlTrAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolOffsetBlBrAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, symbolUBOCount + 1, idSymbolTextureRectAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short4, symbolUBOCount + 1, idSymbolPixelOffsetAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UShort2, symbolUBOCount + 1, idSymbolSizeSdfAttribute},

    // Dynamic
    AttributeInfo{7, gfx::AttributeDataType::Float3, symbolUBOCount + 2, idSymbolProjectedPosVertexAttribute},

    // Opacity
    AttributeInfo{8, gfx::AttributeDataType::Float, symbolUBOCount + 3, idSymbolFadeOpacityVertexAttribute},

    // Data driven
    AttributeInfo{9, gfx::AttributeDataType::Float4, symbolUBOCount + 4, idSymbolColorVertexAttribute},
    AttributeInfo{10, gfx::AttributeDataType::Float4, symbolUBOCount + 4, idSymbolHaloColorVertexAttribute},
    AttributeInfo{11, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolOpacityVertexAttribute},
    AttributeInfo{12, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolHaloWidthVertexAttribute},
    AttributeInfo{13, gfx::AttributeDataType::Float, symbolUBOCount + 4, idSymbolHaloBlurVertexAttribute},
};
const std::array<TextureInfo, 2> SymbolTextAndIconShaderSource::textures = {
    TextureInfo{0, idSymbolImageTexture},
    TextureInfo{1, idSymbolImageIconTexture},
};

} // namespace shaders
} // namespace mbgl
