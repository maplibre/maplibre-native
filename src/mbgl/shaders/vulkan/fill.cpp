#include <mbgl/shaders/vulkan/fill.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Fill

using FillShaderSource = ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 3> FillShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> FillShaderSource::textures = {};

//
// Fill outline

using FillOutlineShaderSource = ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 3> FillOutlineShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillOutlineColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineShaderSource::textures = {};

//
// Fill pattern

using FillPatternShaderSource = ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 4> FillPatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};
const std::array<TextureInfo, 1> FillPatternShaderSource::textures = {
    TextureInfo{0, idFillImageTexture},
};

//
// Fill pattern outline

using FillOutlinePatternShaderSource = ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 4> FillOutlinePatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort4, idFillPatternFromVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort4, idFillPatternToVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
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
} // namespace mbgl
