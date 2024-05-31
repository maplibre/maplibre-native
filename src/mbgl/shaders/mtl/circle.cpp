#include <mbgl/shaders/mtl/circle.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::uniforms = {
    UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, false, sizeof(CircleDrawableUBO), idCircleDrawableUBO},
    UniformBlockInfo{true, true, sizeof(CircleEvaluatedPropsUBO), idCircleEvaluatedPropsUBO},
    UniformBlockInfo{true, false, sizeof(CircleInterpolateUBO), idCircleInterpolateUBO},
};
const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::attributes = {
    AttributeInfo{circleUBOCount + 0, gfx::AttributeDataType::Short2, idCirclePosVertexAttribute},
    AttributeInfo{circleUBOCount + 1, gfx::AttributeDataType::Float4, idCircleColorVertexAttribute},
    AttributeInfo{circleUBOCount + 2, gfx::AttributeDataType::Float2, idCircleRadiusVertexAttribute},
    AttributeInfo{circleUBOCount + 3, gfx::AttributeDataType::Float2, idCircleBlurVertexAttribute},
    AttributeInfo{circleUBOCount + 4, gfx::AttributeDataType::Float2, idCircleOpacityVertexAttribute},
    AttributeInfo{circleUBOCount + 5, gfx::AttributeDataType::Float4, idCircleStrokeColorVertexAttribute},
    AttributeInfo{circleUBOCount + 6, gfx::AttributeDataType::Float2, idCircleStrokeWidthVertexAttribute},
    AttributeInfo{circleUBOCount + 7, gfx::AttributeDataType::Float2, idCircleStrokeOpacityVertexAttribute},
};
const std::array<TextureInfo, 0> ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>::textures = {};

} // namespace shaders
} // namespace mbgl
