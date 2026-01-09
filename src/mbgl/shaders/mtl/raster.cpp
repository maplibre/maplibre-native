#include <mbgl/shaders/mtl/raster.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

using RasterShaderSource = ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>;

const std::array<AttributeInfo, 2> RasterShaderSource::attributes = {
    AttributeInfo{rasterUBOCount + 0, gfx::AttributeDataType::Short2, idRasterPosVertexAttribute},
    AttributeInfo{rasterUBOCount + 1, gfx::AttributeDataType::Short2, idRasterTexturePosVertexAttribute},
};
const std::array<TextureInfo, 2> RasterShaderSource::textures = {
    TextureInfo{0, idRasterImage0Texture},
    TextureInfo{1, idRasterImage1Texture},
};

} // namespace shaders
} // namespace mbgl
