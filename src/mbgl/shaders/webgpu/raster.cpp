#include <mbgl/shaders/webgpu/raster.hpp>
#include <mbgl/shaders/shader_defines.hpp>
#include <mbgl/shaders/raster_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

using RasterShaderSource = ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::WebGPU>;

const std::array<AttributeInfo, 2> RasterShaderSource::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idRasterPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Short2, idRasterTexturePosVertexAttribute},
};
const std::array<TextureInfo, 2> RasterShaderSource::textures = {TextureInfo{0, idRasterImage0Texture},
                                                                 TextureInfo{1, idRasterImage1Texture}};

} // namespace shaders
} // namespace mbgl
