// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/raster.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal>::reflectionData = {
    "RasterShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Short2, 1, "a_texture_pos"},
    },
    {
        UniformBlockInfo{2, true, true, sizeof(RasterDrawableUBO), "RasterDrawableUBO"},
    },
    {
        TextureInfo{0, "u_image0"},
        TextureInfo{1, "u_image1"},
    },
    {
        BuiltIn::Prelude,
    },
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
