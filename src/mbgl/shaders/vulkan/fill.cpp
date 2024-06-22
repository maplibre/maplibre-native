#include <mbgl/shaders/vulkan/fill.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan>::uniforms = {
	UniformBlockInfo{true, false, sizeof(FillDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{true, false, sizeof(FillInterpolateUBO), idFillInterpolateUBO},
	UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan>::attributes = {
	AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillColorVertexAttribute},
	AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, false, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, false, sizeof(FillOutlineDrawableUBO), idFillDrawableUBO},
    UniformBlockInfo{true, false, sizeof(FillOutlineInterpolateUBO), idFillInterpolateUBO},
    UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillOutlineColorVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},
};

} // namespace shaders
} // namespace mbgl
