#include <mbgl/shaders/vulkan/background.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(BackgroundDrawableUBO), idBackgroundDrawableUBO},
    UniformBlockInfo{false, true, sizeof(BackgroundLayerUBO), idBackgroundLayerUBO},
};
const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idBackgroundPosVertexAttribute},
};

const std::array<UniformBlockInfo, 3>
    ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Vulkan>::uniforms = {
        UniformBlockInfo{false, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
        UniformBlockInfo{true, false, sizeof(BackgroundPatternDrawableUBO), idBackgroundDrawableUBO},
        UniformBlockInfo{true, true, sizeof(BackgroundPatternLayerUBO), idBackgroundLayerUBO},
};
const std::array<AttributeInfo, 1>
    ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Vulkan>::attributes = {
        AttributeInfo{0, gfx::AttributeDataType::Short2, idBackgroundPosVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Vulkan>::textures =
    {TextureInfo{0, idBackgroundImageTexture}};

} // namespace shaders
} // namespace mbgl
