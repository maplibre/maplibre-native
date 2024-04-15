#include <mbgl/shaders/mtl/raster.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, false, sizeof(RasterDrawableUBO), idRasterDrawableUBO},
    UniformBlockInfo{true, true, sizeof(RasterEvaluatedPropsUBO), idRasterEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{rasterUBOCount + 0, gfx::AttributeDataType::Short2, idRasterPosVertexAttribute},
    AttributeInfo{rasterUBOCount + 1, gfx::AttributeDataType::Short2, idRasterTexturePosVertexAttribute},
};
const std::array<TextureInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idRasterImage0Texture},
    TextureInfo{1, idRasterImage1Texture},
};

} // namespace shaders
} // namespace mbgl
