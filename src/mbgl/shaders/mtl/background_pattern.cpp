#include <mbgl/shaders/mtl/background_pattern.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 1>
    ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Float3, idBackgroundPosVertexAttribute},
};
const std::array<UniformBlockInfo, 2>
    ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{1, true, false, sizeof(BackgroundDrawableUBO), idBackgroundDrawableUBO},
        UniformBlockInfo{2, true, true, sizeof(BackgroundPatternLayerUBO), idBackgroundLayerUBO},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idBackgroundImageTexture}};

} // namespace shaders
} // namespace mbgl
