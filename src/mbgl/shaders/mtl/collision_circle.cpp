// Generated code, do not modify this file!
// NOLINTBEGIN
#include <mbgl/shaders/mtl/collision_circle.hpp>

namespace mbgl {
namespace shaders {

const ReflectionData ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal>::reflectionData = {
    "CollisionCircleShader",
    "vertexMain",
    "fragmentMain",
    {
        AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        AttributeInfo{1, gfx::AttributeDataType::Short2, 1, "a_anchor_pos"},
        AttributeInfo{2, gfx::AttributeDataType::Short2, 1, "a_extrude"},
        AttributeInfo{3, gfx::AttributeDataType::UByte2, 1, "a_placed"},
    },
    {
        UniformBlockInfo{4, true, true, sizeof(CollisionUBO), "CollisionUBO"},
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
