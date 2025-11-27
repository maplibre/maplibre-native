#include <mbgl/shaders/webgpu/fill.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Fill

using FillShaderSource = ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> FillShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> FillShaderSource::textures = {};

//
// Fill outline

using FillOutlineShaderSource = ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 3> FillOutlineShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillOutlineColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineShaderSource::textures = {};

//
// Fill pattern

using FillPatternShaderSource = ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 4> FillPatternShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> FillPatternShaderSource::textures = {TextureInfo{0, idFillImageTexture}};

//
// Fill outline pattern

using FillOutlinePatternShaderSource = ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 4> FillOutlinePatternShaderSource::attributes = {
    AttributeInfo{4, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> FillOutlinePatternShaderSource::textures = {TextureInfo{0, idFillImageTexture}};

//
// Fill outline triangulated

using FillOutlineTriangulatedShaderSource =
    ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> FillOutlineTriangulatedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineTriangulatedShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
