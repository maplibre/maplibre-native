#include <mln/shaders/mtl/line.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

//
// Line

using LineShaderSource = ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> LineShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, lineUBOCount + 0, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, lineUBOCount + 0, idLineDataVertexAttribute},

    // Shared per-vertex index into the line feature table.
    AttributeInfo{2, gfx::AttributeDataType::UInt, lineUBOCount + 2, idLineColorVertexAttribute},
};
const std::array<TextureInfo, 0> LineShaderSource::textures = {};

//
// Line gradient

using LineGradientShaderSource = ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> LineGradientShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, lineUBOCount + 0, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, lineUBOCount + 0, idLineDataVertexAttribute},

    AttributeInfo{2, gfx::AttributeDataType::UInt, lineUBOCount + 2, idLineColorVertexAttribute},
};
const std::array<TextureInfo, 1> LineGradientShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
};

//
// Line pattern

using LinePatternShaderSource = ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> LinePatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, lineUBOCount + 0, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, lineUBOCount + 0, idLineDataVertexAttribute},

    AttributeInfo{2, gfx::AttributeDataType::UInt, lineUBOCount + 2, idLineColorVertexAttribute},
};
const std::array<TextureInfo, 1> LinePatternShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
};

//
// Line SDF

using LineSDFShaderSource = ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> LineSDFShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, lineUBOCount + 0, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, lineUBOCount + 0, idLineDataVertexAttribute},

    AttributeInfo{2, gfx::AttributeDataType::UInt, lineUBOCount + 2, idLineColorVertexAttribute},
};
const std::array<TextureInfo, 1> LineSDFShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
};

//
// Line gradient SDF

using LineGradientSDFShaderSource = ShaderSource<BuiltIn::LineGradientSDFShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> LineGradientSDFShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, lineUBOCount + 0, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, lineUBOCount + 0, idLineDataVertexAttribute},

    AttributeInfo{2, gfx::AttributeDataType::UInt, lineUBOCount + 2, idLineColorVertexAttribute},
};
const std::array<TextureInfo, 2> LineGradientSDFShaderSource::textures = {
    TextureInfo{0, idLineImageTexture},
    TextureInfo{1, idLineDashTexture},
};

} // namespace shaders
} // namespace mln
