#include <mbgl/shaders/vulkan/raster.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(RasterDrawableUBO), idRasterDrawableUBO},
    UniformBlockInfo{true, true, sizeof(RasterEvaluatedPropsUBO), idRasterEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idRasterPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, idRasterTexturePosVertexAttribute},
};
const std::array<TextureInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Vulkan>::textures = {
    TextureInfo{0, idRasterImage0Texture},
    TextureInfo{1, idRasterImage1Texture},
};

} // namespace shaders
} // namespace mbgl
