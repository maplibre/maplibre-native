#include <mbgl/shaders/webgpu/fill.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

//
// Fill

using FillShaderSource = ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> FillShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idFillPosVertexAttribute},
};
const std::array<TextureInfo, 0> FillShaderSource::textures = {};

//
// Fill outline

using FillOutlineShaderSource = ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> FillOutlineShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float2, idFillPosVertexAttribute},
};
const std::array<TextureInfo, 0> FillOutlineShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl