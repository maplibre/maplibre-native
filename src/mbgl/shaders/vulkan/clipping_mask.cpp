#include <mbgl/shaders/vulkan/clipping_mask.hpp>

namespace mbgl {
namespace shaders {

using ClippingMaskShaderSource = ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Vulkan>;

const std::array<AttributeInfo, 1> ClippingMaskShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idClippingMaskPosVertexAttribute},
};
const std::array<TextureInfo, 0> ClippingMaskShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
