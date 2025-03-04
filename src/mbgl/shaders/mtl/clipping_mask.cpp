#include <mbgl/shaders/mtl/clipping_mask.hpp>

namespace mbgl {
namespace shaders {

using ClippingMaskShaderSource = ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 1> ClippingMaskShaderSource::attributes = {
    AttributeInfo{clippingMaskUBOCount + 0, gfx::AttributeDataType::Float3, idClippingMaskPosVertexAttribute},
};
const std::array<TextureInfo, 0> ClippingMaskShaderSource::textures = {};

} // namespace shaders
} // namespace mbgl
