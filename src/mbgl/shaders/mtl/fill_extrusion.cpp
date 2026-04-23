#include <mbgl/shaders/mtl/fill_extrusion.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 4> FillExtrusionShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},

    // Data driven
    AttributeInfo{1, gfx::AttributeDataType::Float4, fillExtrusionUBOCount + 1, idFillExtrusionColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShaderSource::textures = {};

//
// Fill extrusion instanced

using FillExtrusionInstancedShaderSource = ShaderSource<BuiltIn::FillExtrusionInstancedShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> FillExtrusionInstancedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
};
const std::array<AttributeInfo, 5> FillExtrusionInstancedShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 1, idFillExtrusionOutlinePosAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 1, idFillExtrusionEdDiscardAttribute},
    
    // Data driven
    AttributeInfo{3, gfx::AttributeDataType::Float4, fillExtrusionUBOCount + 2, idFillExtrusionColorVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 2, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 2, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionInstancedShaderSource::textures = {};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 5> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},

    // Data driven
    AttributeInfo{1, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{
        3, gfx::AttributeDataType::UShort4, fillExtrusionUBOCount + 1, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{
        4, gfx::AttributeDataType::UShort4, fillExtrusionUBOCount + 1, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

//
// Fill extrusion pattern instanced

using FillExtrusionPatternInstancedShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternInstancedShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> FillExtrusionPatternInstancedShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
};
const std::array<AttributeInfo, 6> FillExtrusionPatternInstancedShaderSource::instanceAttributes = {
    AttributeInfo{1, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 1, idFillExtrusionOutlinePosAttribute},
    AttributeInfo{2, gfx::AttributeDataType::UShort2, fillExtrusionUBOCount + 1, idFillExtrusionEdDiscardAttribute},
    
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
} // namespace mbgl
