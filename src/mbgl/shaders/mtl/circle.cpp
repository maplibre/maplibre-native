#include <mbgl/shaders/mtl/circle.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, 1, "a_pos"},
    AttributeInfo{1, gfx::AttributeDataType::Float4, 1, "a_color"},
    AttributeInfo{2, gfx::AttributeDataType::Float2, 1, "a_radius"},
    AttributeInfo{3, gfx::AttributeDataType::Float2, 1, "a_blur"},
    AttributeInfo{4, gfx::AttributeDataType::Float2, 1, "a_opacity"},
    AttributeInfo{5, gfx::AttributeDataType::Float4, 1, "a_stroke_color"},
    AttributeInfo{6, gfx::AttributeDataType::Float2, 1, "a_stroke_width"},
    AttributeInfo{7, gfx::AttributeDataType::Float2, 1, "a_stroke_opacity"},
};
const std::array<UniformBlockInfo, 6> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{8, true, false, sizeof(CircleDrawableUBO), "CircleDrawableUBO"},
    UniformBlockInfo{9, true, true, sizeof(CirclePaintParamsUBO), "CirclePaintParamsUBO"},
    UniformBlockInfo{10, true, true, sizeof(CircleEvaluatedPropsUBO), "CircleEvaluatedPropsUBO"},
    UniformBlockInfo{11, true, false, sizeof(CircleInterpolateUBO), "CircleInterpolateUBO"},
    UniformBlockInfo{12, true, true, sizeof(CirclePermutationUBO), "CirclePermutationUBO"},
    UniformBlockInfo{13, true, false, sizeof(ExpressionInputsUBO), "ExpressionInputsUBO"},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
