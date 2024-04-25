#include <mbgl/shaders/mtl/background_pattern.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 3>
    ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{false, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, false, sizeof(BackgroundPatternDrawableUBO), idBackgroundDrawableUBO},
        UniformBlockInfo{true, true, sizeof(BackgroundPatternLayerUBO), idBackgroundLayerUBO},
};
const std::array<AttributeInfo, 1>
    ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>::attributes = {
        AttributeInfo{backgroundUBOCount + 0, gfx::AttributeDataType::Float3, idBackgroundPosVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idBackgroundImageTexture}};

} // namespace shaders
} // namespace mbgl
