// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/background.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal>::reflectionData = {
    "BackgroundShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Float3, 1, "a_pos"},
    },
    {
        UniformBlockInfo{1, true, true, sizeof(BackgroundLayerUBO), "BackgroundLayerUBO"},
        UniformBlockInfo{2, true, false, sizeof(BackgroundDrawableUBO), "BackgroundDrawableUBO"},
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
