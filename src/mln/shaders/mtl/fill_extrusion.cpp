#include <mln/shaders/mtl/fill_extrusion.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 6> FillExtrusionShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 0, idFillExtrusionDecimalsEdAttribute},

    // Data driven
    AttributeInfo{2, gfx::AttributeDataType::Float4, fillExtrusionUBOCount + 1, idFillExtrusionColorVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionHeightVertexAttribute},

    // Polygon centroid for the terrain elevation lookup. Interleaved in the same
    // shared vertex buffer as pos/decimals_ed, so it shares their buffer index.
    AttributeInfo{5, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionCentroidVertexAttribute},
};
// DEM for terrain elevation, sampled in the vertex stage (Metal binds textures to
// both stages). The index is the MSL texture/sampler slot.
const std::array<TextureInfo, 1> FillExtrusionShaderSource::textures = {
    TextureInfo{0, idFillExtrusionDEMTexture},
};

//
// Fill extrusion instanced

using FillExtrusionInstancedShaderSource =
    ShaderSource<BuiltIn::FillExtrusionInstancedShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> FillExtrusionInstancedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
};
const std::array<AttributeInfo, 5> FillExtrusionInstancedShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 1, idFillExtrusionOutlinePosAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 1, idFillExtrusionDecimalsEdAttribute},

    // Data driven
    AttributeInfo{3, gfx::AttributeDataType::Float4, fillExtrusionUBOCount + 2, idFillExtrusionColorVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 2, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 2, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionInstancedShaderSource::textures = {
    TextureInfo{0, idFillExtrusionDEMTexture},
};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 6> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 0, idFillExtrusionDecimalsEdAttribute},

    // Data driven
    AttributeInfo{2, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{
        4, gfx::AttributeDataType::UShort4, fillExtrusionUBOCount + 1, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{
        5, gfx::AttributeDataType::UShort4, fillExtrusionUBOCount + 1, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

//
// Fill extrusion pattern instanced

using FillExtrusionPatternInstancedShaderSource =
    ShaderSource<BuiltIn::FillExtrusionPatternInstancedShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> FillExtrusionPatternInstancedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
};
const std::array<AttributeInfo, 6> FillExtrusionPatternInstancedShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 1, idFillExtrusionOutlinePosAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 1, idFillExtrusionDecimalsEdAttribute},

    // Data driven
    AttributeInfo{3, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 2, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 2, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{
        5, gfx::AttributeDataType::UShort4, fillExtrusionUBOCount + 2, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{
        6, gfx::AttributeDataType::UShort4, fillExtrusionUBOCount + 2, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternInstancedShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mln
