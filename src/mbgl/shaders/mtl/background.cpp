#include <mbgl/shaders/mtl/background.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Float3, "a_pos"},
};
const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{1, true, false, sizeof(BackgroundDrawableUBO), "BackgroundDrawableUBO"},
    UniformBlockInfo{2, false, true, sizeof(BackgroundLayerUBO), "BackgroundLayerUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
