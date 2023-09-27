#include <mbgl/shaders/mtl/clipping_mask.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo,1> ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float3, 1, "a_pos"},
};
const std::array<UniformBlockInfo,1> ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{1, true, false, sizeof(ClipUBO), "ClipUBO"},
};
const std::array<TextureInfo,0> ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
