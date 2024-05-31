#include <mbgl/shaders/mtl/clipping_mask.hpp>

namespace mbgl {
namespace shaders {

using ShaderType = ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal>;

const std::array<UniformBlockInfo, 1> ShaderType::uniforms = {
    UniformBlockInfo{true, false, sizeof(ClipUBO), idClippingMaskUBO},
};
const std::array<AttributeInfo, 1> ShaderType::attributes = {
    AttributeInfo{clippingMaskUBOCount + 0, gfx::AttributeDataType::Float3, idClippingMaskPosVertexAttribute},
};
const std::array<TextureInfo, 0> ShaderType::textures = {};

} // namespace shaders
} // namespace mbgl
