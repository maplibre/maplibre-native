#include <mbgl/shaders/vulkan/circle.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, false, sizeof(CircleDrawableUBO), idCircleDrawableUBO},
    UniformBlockInfo{true, true, sizeof(CircleEvaluatedPropsUBO), idCircleEvaluatedPropsUBO},
    UniformBlockInfo{true, false, sizeof(CircleInterpolateUBO), idCircleInterpolateUBO},
};
const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idCirclePosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idCircleColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idCircleRadiusVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idCircleBlurVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idCircleOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float4, idCircleStrokeColorVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idCircleStrokeWidthVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idCircleStrokeOpacityVertexAttribute},
};

} // namespace shaders
} // namespace mbgl
