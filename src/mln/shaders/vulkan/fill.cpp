#include <mln/shaders/vulkan/fill.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/fill_layer_ubo.hpp>

namespace mln {
namespace shaders {

//
// Fill

using FillShaderSource = ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 5> FillShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute, idFillPosVertexDrawableUBO},

    // used only in rounded corners
    AttributeInfo{1, gfx::AttributeDataType::Short2, idFillNeighborVertexAttribute, idFillNeighborVertexDrawableUBO},

    // sometimes uniforms
    AttributeInfo{2, gfx::AttributeDataType::Float4, idFillColorVertexAttribute, idFillDataVertexDrawableUBO},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute, idFillDataVertexDrawableUBO},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idFillFadeZoomVertexAttribute, idFillDataVertexDrawableUBO},
};
const std::array<AttributeInfo, 1> FillShaderSource::instanceAttributes = {
    // used only in rounded corners
    AttributeInfo{4, gfx::AttributeDataType::UInt3, idFillTriangleIndicesAttribute},
};
const std::array<TextureInfo, 0> FillShaderSource::textures = {};

//
// Fill outline

using FillOutlineShaderSource = ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 4> FillOutlineShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},

    // sometimes uniforms
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillOutlineColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillFadeZoomVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineShaderSource::textures = {};

//
// Fill pattern

using FillPatternShaderSource = ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 7> FillPatternShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute, idFillPosVertexDrawableUBO},

    // used only in rounded corners
    AttributeInfo{1, gfx::AttributeDataType::Short2, idFillNeighborVertexAttribute, idFillNeighborVertexDrawableUBO},

    // sometimes uniforms
    AttributeInfo{2, gfx::AttributeDataType::Float4, idFillColorVertexAttribute, idFillDataVertexDrawableUBO},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute, idFillDataVertexDrawableUBO},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute, idFillDataVertexDrawableUBO},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute, idFillDataVertexDrawableUBO},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idFillFadeZoomVertexAttribute, idFillDataVertexDrawableUBO},
};
const std::array<AttributeInfo, 1> FillPatternShaderSource::instanceAttributes = {
    // used only in rounded corners
    AttributeInfo{6, gfx::AttributeDataType::UInt3, idFillTriangleIndicesAttribute},
};
const std::array<TextureInfo, 1> FillPatternShaderSource::textures = {
    TextureInfo{0, idFillImageTexture},
};

//
// Fill pattern outline

using FillOutlinePatternShaderSource = ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 6> FillOutlinePatternShaderSource::attributes = {
    // always attributes
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},

    // sometimes uniforms
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idFillFadeZoomVertexAttribute},
};
const std::array<TextureInfo, 1> FillOutlinePatternShaderSource::textures = {
    TextureInfo{0, idFillImageTexture},
};

//
// Fill outline triangulated

using FillOutlineTriangulatedShaderSource =
    ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 2> FillOutlineTriangulatedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineTriangulatedShaderSource::textures = {};

} // namespace shaders
} // namespace mln
