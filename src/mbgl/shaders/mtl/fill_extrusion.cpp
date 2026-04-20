#include <mbgl/shaders/mtl/fill_extrusion.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 5> FillExtrusionShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short4, fillExtrusionUBOCount + 0, idFillExtrusionNormalEdVertexAttribute},
    
    // Data driven
    AttributeInfo{2, gfx::AttributeDataType::Float4, fillExtrusionUBOCount + 1, idFillExtrusionColorVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShaderSource::textures = {};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 6> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, fillExtrusionUBOCount + 0, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short4, fillExtrusionUBOCount + 0, idFillExtrusionNormalEdVertexAttribute},
    
    // Data driven
    AttributeInfo{2, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float, fillExtrusionUBOCount + 1, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort4, fillExtrusionUBOCount + 1, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::UShort4, fillExtrusionUBOCount + 1, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mbgl
