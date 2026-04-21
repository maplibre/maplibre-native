#include <mbgl/shaders/mtl/fill.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Fill

using FillShaderSource = ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> FillShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillUBOCount + 0, idFillPosVertexAttribute},
    
    // Data driven
    AttributeInfo{1, gfx::AttributeDataType::Float4, fillUBOCount + 1, idFillColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, fillUBOCount + 1, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> FillShaderSource::textures = {};

//
// Fill outline

using FillOutlineShaderSource = ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> FillOutlineShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillUBOCount + 0, idFillPosVertexAttribute},
    
    // Data driven
    AttributeInfo{1, gfx::AttributeDataType::Float4, fillUBOCount + 1, idFillOutlineColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, fillUBOCount + 1, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineShaderSource::textures = {};

//
// Fill pattern

using FillPatternShaderSource = ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 4> FillPatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillUBOCount + 0, idFillPosVertexAttribute},
    
    // Data driven
    AttributeInfo{1, gfx::AttributeDataType::UShort4, fillUBOCount + 1, idFillPatternFromVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort4, fillUBOCount + 1, idFillPatternToVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, fillUBOCount + 1, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> FillPatternShaderSource::textures = {
    TextureInfo{0, idFillImageTexture},
};

//
// Fill pattern outline

using FillOutlinePatternShaderSource = ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 4> FillOutlinePatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillUBOCount + 0, idFillPosVertexAttribute},
    
    // Data driven
    AttributeInfo{1, gfx::AttributeDataType::UShort4, fillUBOCount + 1, idFillPatternFromVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort4, fillUBOCount + 1, idFillPatternToVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, fillUBOCount + 1, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> FillOutlinePatternShaderSource::textures = {
    TextureInfo{0, idFillImageTexture},
};

//
// Fill outline triangulated

using FillOutlineTriangulatedShaderSource =
    ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> FillOutlineTriangulatedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillUBOCount + 0, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, fillUBOCount + 0, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineTriangulatedShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
