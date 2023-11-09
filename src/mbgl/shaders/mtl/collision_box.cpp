// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/collision_box.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Metal>::reflectionData = {
    "CollisionBoxShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Short2, 1, "a_anchor_pos"},
        AttributeInfo{2, gfx::AttributeDataType::Short2, 1, "a_extrude"},
        AttributeInfo{3, gfx::AttributeDataType::UByte2, 1, "a_placed"},
        AttributeInfo{4, gfx::AttributeDataType::Float2, 1, "a_shift"},
    },
    {
        UniformBlockInfo{5, true, true, sizeof(CollisionUBO), "CollisionUBO"},
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
