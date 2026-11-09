#include <mln/shaders/webgpu/fill_extrusion.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/fill_extrusion_layer_ubo.hpp>

namespace mln {
namespace shaders {

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 4> FillExtrusionShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort2, idFillExtrusionDecimalsEdAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short2, idFillExtrusionNormal2DVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UInt, idFillExtrusionBaseVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShaderSource::textures = {};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 4> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::UShort2, idFillExtrusionDecimalsEdAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Short2, idFillExtrusionNormal2DVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::UInt, idFillExtrusionBaseVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mln
