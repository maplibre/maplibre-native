#include <mbgl/shaders/mtl/raster.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Short2, "a_texture_pos"},
};
const std::array<UniformBlockInfo, 1> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{2, true, true, sizeof(RasterDrawableUBO), idRasterDrawableUBO},
};
const std::array<TextureInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idRasterImage0Texture},
    TextureInfo{1, idRasterImage1Texture},
};

} // namespace shaders
} // namespace mbgl
