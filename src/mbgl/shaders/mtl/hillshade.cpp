// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/hillshade.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Metal>::reflectionData = {
    "HillshadeShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Short2, 1, "a_texture_pos"},
    },
    {
        UniformBlockInfo{2, true, true, sizeof(HillshadeDrawableUBO), "HillshadeDrawableUBO"},
        UniformBlockInfo{3, false, true, sizeof(HillshadeEvaluatedPropsUBO), "HillshadeEvaluatedPropsUBO"},
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
