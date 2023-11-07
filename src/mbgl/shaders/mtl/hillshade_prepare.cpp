// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/hillshade_prepare.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Metal>::reflectionData = {
    "HillshadePrepareShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Short2, 1, "a_texture_pos"},
    },
    {
        UniformBlockInfo{2, true, true, sizeof(HillshadePrepareDrawableUBO), "HillshadePrepareDrawableUBO"},
    },
    {
        TextureInfo{0, "u_image"},
    },
    {
        BuiltIn::Prelude,
    },
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
