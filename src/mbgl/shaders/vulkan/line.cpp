#include <mbgl/shaders/vulkan/line.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 4> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Vulkan>::uniforms = {
    UniformBlockInfo{true, true, sizeof(GlobalPaintParamsUBO), idGlobalPaintParamsUBO},
    UniformBlockInfo{true, false, sizeof(LineDrawableUBO), idLineDrawableUBO},
    UniformBlockInfo{true, false, sizeof(LineInterpolationUBO), idLineInterpolationUBO},
    UniformBlockInfo{true, true, sizeof(LineEvaluatedPropsUBO), idLineEvaluatedPropsUBO},
};
const std::array<AttributeInfo, 8> ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Vulkan>::attributes = {
    AttributeInfo{0, gfx::AttributeDataType::Short2, idLinePosNormalVertexAttribute},
    AttributeInfo{1, gfx::AttributeDataType::UByte4, idLineDataVertexAttribute},
    AttributeInfo{2, gfx::AttributeDataType::Float4, idLineColorVertexAttribute},
    AttributeInfo{3, gfx::AttributeDataType::Float2, idLineBlurVertexAttribute},
    AttributeInfo{4, gfx::AttributeDataType::Float2, idLineOpacityVertexAttribute},
    AttributeInfo{5, gfx::AttributeDataType::Float2, idLineGapWidthVertexAttribute},
    AttributeInfo{6, gfx::AttributeDataType::Float2, idLineOffsetVertexAttribute},
    AttributeInfo{7, gfx::AttributeDataType::Float2, idLineWidthVertexAttribute},
};

} // namespace shaders
} // namespace mbgl
