// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/clipping_mask.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal>::reflectionData = {
    "ClippingMaskProgram",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Float3, 1, "a_pos"},
    },
    {
        UniformBlockInfo{1, true, false, sizeof(ClipUBO), "ClipUBO"},
    },
    {

    },
    {
        BuiltIn::Prelude,
    },
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
