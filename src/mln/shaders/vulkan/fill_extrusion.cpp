#include <mln/shaders/vulkan/fill_extrusion.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/fill_extrusion_layer_ubo.hpp>

namespace mln {
namespace shaders {

// idFillExtrusionInstancedDrawableUBO is shared between instanced and non-instanced drawables

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 6> FillExtrusionShaderSource::attributes = {
    AttributeInfo{
        0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute, idFillExtrusionInstancedDrawableUBO},
    AttributeInfo{
        1, gfx::AttributeDataType::UShort2, idFillExtrusionDecimalsEdAttribute, idFillExtrusionInstancedDrawableUBO},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idFillExtrusionColorVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short2, idFillExtrusionCentroidVertexAttribute},
};
// DEM for terrain elevation. The GLSL binding is the texture-id slot (1), not
// this array's order - see the shader source note.
const std::array<TextureInfo, 1> FillExtrusionShaderSource::textures = {
    TextureInfo{1, idFillExtrusionDEMTexture},
};

//
// Fill extrusion instanced

using FillExtrusionInstancedShaderSource =
    ShaderSource<BuiltIn::FillExtrusionInstancedShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> FillExtrusionInstancedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
};
const std::array<AttributeInfo, 5> FillExtrusionInstancedShaderSource::instanceAttributes = {
    AttributeInfo{
        1, gfx::AttributeDataType::Short2, idFillExtrusionOutlinePosAttribute, idFillExtrusionInstancedDrawableUBO},
    AttributeInfo{
        2, gfx::AttributeDataType::UShort2, idFillExtrusionDecimalsEdAttribute, idFillExtrusionInstancedDrawableUBO},

    // Data driven
    AttributeInfo{3, gfx::AttributeDataType::Float4, idFillExtrusionColorVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionInstancedShaderSource::textures = {
    TextureInfo{1, idFillExtrusionDEMTexture},
};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 6> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{
        0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute, idFillExtrusionInstancedDrawableUBO},
    AttributeInfo{
        1, gfx::AttributeDataType::UShort2, idFillExtrusionDecimalsEdAttribute, idFillExtrusionInstancedDrawableUBO},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

//
// Fill extrusion pattern instanced

using FillExtrusionPatternInstancedShaderSource =
    ShaderSource<BuiltIn::FillExtrusionPatternInstancedShader, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> FillExtrusionPatternInstancedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
};
const std::array<AttributeInfo, 6> FillExtrusionPatternInstancedShaderSource::instanceAttributes = {
    AttributeInfo{
        1, gfx::AttributeDataType::Short2, idFillExtrusionOutlinePosAttribute, idFillExtrusionInstancedDrawableUBO},
    AttributeInfo{
        2, gfx::AttributeDataType::UShort2, idFillExtrusionDecimalsEdAttribute, idFillExtrusionInstancedDrawableUBO},

    // Data driven
    AttributeInfo{3, gfx::AttributeDataType::Float2, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UShort4, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternInstancedShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mln
