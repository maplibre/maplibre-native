#include <mln/shaders/mtl/fill_extrusion.hpp>
#include <mln/shaders/shader_defines.hpp>

namespace mln {
namespace shaders {

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> FillExtrusionShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 0, idFillExtrusionDecimalsEdAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UInt, fillExtrusionUBOCount + 1, idFillExtrusionBaseVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShaderSource::textures = {};

//
// Fill extrusion instanced

using FillExtrusionInstancedShaderSource =
    ShaderSource<BuiltIn::FillExtrusionInstancedShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> FillExtrusionInstancedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
};
const std::array<AttributeInfo, 3> FillExtrusionInstancedShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 1, idFillExtrusionOutlinePosAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 1, idFillExtrusionDecimalsEdAttribute},
    AttributeInfo{3, gfx::AttributeDataType::UInt, fillExtrusionUBOCount + 2, idFillExtrusionBaseVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionInstancedShaderSource::textures = {};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 3> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 0, idFillExtrusionDecimalsEdAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UInt, fillExtrusionUBOCount + 1, idFillExtrusionBaseVertexAttribute},
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
const std::array<AttributeInfo, 3> FillExtrusionPatternInstancedShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 1, idFillExtrusionOutlinePosAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 1, idFillExtrusionDecimalsEdAttribute},
    AttributeInfo{3, gfx::AttributeDataType::UInt, fillExtrusionUBOCount + 2, idFillExtrusionBaseVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternInstancedShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mln
