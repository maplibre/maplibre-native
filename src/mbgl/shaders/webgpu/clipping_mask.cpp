#include <mbgl/shaders/webgpu/clipping_mask.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using ClippingMaskShaderSource = ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 1> ClippingMaskShaderSource::attributes = {
    AttributeInfo{2, gfx::AttributeDataType::Short2, idClippingMaskPosVertexAttribute},
};
const std::array<TextureInfo, 0> ClippingMaskShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
