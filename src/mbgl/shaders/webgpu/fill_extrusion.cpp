#include <mbgl/shaders/webgpu/fill_extrusion.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Fill extrusion

using FillExtrusionShaderSource = ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> FillExtrusionShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillExtrusionNormalVertexAttribute},
};
const std::array<TextureInfo, 0> FillExtrusionShaderSource::textures = {};

//
// Fill extrusion pattern

using FillExtrusionPatternShaderSource = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> FillExtrusionPatternShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idFillExtrusionPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillExtrusionNormalVertexAttribute},
};
const std::array<TextureInfo, 1> FillExtrusionPatternShaderSource::textures = {
    TextureInfo{0, idFillExtrusionPatternTexture}
};

} // namespace shaders
} // namespace mbgl