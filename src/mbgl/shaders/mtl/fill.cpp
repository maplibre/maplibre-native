#include <mbgl/shaders/mtl/fill.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Fill

using FillShaderSource = ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> FillShaderSource::attributes = {
    AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::Float4, idFillColorVertexAttribute},
    AttributeInfo{fillUBOCount + 2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> FillShaderSource::textures = {};

//
// Fill outline

using FillOutlineShaderSource = ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> FillOutlineShaderSource::attributes = {
    AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::Float4, idFillOutlineColorVertexAttribute},
    AttributeInfo{fillUBOCount + 2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineShaderSource::textures = {};

//
// Fill pattern

using FillPatternShaderSource = ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 4> FillPatternShaderSource::attributes = {
    AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{fillUBOCount + 2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{fillUBOCount + 3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> FillPatternShaderSource::textures = {
    TextureInfo{0, idFillImageTexture},
};

//
// Fill pattern outline

using FillOutlinePatternShaderSource = ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 4> FillOutlinePatternShaderSource::attributes = {
    AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{fillUBOCount + 2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{fillUBOCount + 3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> FillOutlinePatternShaderSource::textures = {
    TextureInfo{0, idFillImageTexture},
};

//
// Fill outline triangulated

using FillOutlineTriangulatedShaderSource =
    ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> FillOutlineTriangulatedShaderSource::attributes = {
    AttributeInfo{fillUBOCount + 0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{fillUBOCount + 1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineTriangulatedShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
