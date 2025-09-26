#include <mbgl/shaders/webgpu/fill_extrusion.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 5> FillExtrusionShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float4, idFillExtrusionColorVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idFillExtrusionHeightVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShaderSource::textures = {};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 6> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{3, gfx::AttributeDataType::Short2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Short4, idFillExtrusionNormalEdVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idFillExtrusionBaseVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idFillExtrusionHeightVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::UShort4, idFillExtrusionPatternFromVertexAttribute},
    AttributeInfo{8, gfx::AttributeDataType::UShort4, idFillExtrusionPatternToVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternShaderSource::textures = {
    TextureInfo{0, idFillExtrusionImageTexture},
};

} // namespace shaders
} // namespace mbgl
