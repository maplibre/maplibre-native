#include <mbgl/shaders/mtl/raster.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Short2, 1, "a_texture_pos"},
};
const std::array<UniformBlockInfo, 1> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{2, true, true, sizeof(RasterDrawableUBO), "RasterDrawableUBO"},
};
const std::array<TextureInfo, 2> ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, "u_image0"},
    TextureInfo{1, "u_image1"},
};

} // namespace shaders
} // namespace mbgl
