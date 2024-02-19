#include <mbgl/shaders/mtl/circle.hpp>

namespace mbgl {
namespace shaders {

const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idCirclePosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idCircleColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idCircleRadiusVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idCircleBlurVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idCircleOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float4, idCircleStrokeColorVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idCircleStrokeWidthVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idCircleStrokeOpacityVertexAttribute},
};
const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{8, true, false, sizeof(CircleDrawableUBO), idCircleDrawableUBO},
    UniformBlockInfo{9, true, true, sizeof(CirclePaintParamsUBO), idCirclePaintParamsUBO},
    UniformBlockInfo{10, true, true, sizeof(CircleEvaluatedPropsUBO), idCircleEvaluatedPropsUBO},
    UniformBlockInfo{11, true, false, sizeof(CircleInterpolateUBO), idCircleInterpolateUBO},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
