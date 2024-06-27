#include <mbgl/shaders/vulkan/circle.hpp>

namespace mbgl {
namespace shaders {

#define CREATE_TEST_SHADER_(x)                                                                                      \
const std::array<UniformBlockInfo, 2> ShaderSource<BuiltIn::x, gfx::Backend::Type::Vulkan>::uniforms = {			\
	UniformBlockInfo{true, false, sizeof(FillDrawableUBO), idFillDrawableUBO},										\
	UniformBlockInfo{true, true, sizeof(FillEvaluatedPropsUBO), idFillEvaluatedPropsUBO},							\
};																													\
const std::array<AttributeInfo, 3> ShaderSource<BuiltIn::x, gfx::Backend::Type::Vulkan>::attributes = {				\
	AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},										\
    AttributeInfo{1, gfx::AttributeDataType::Float4, idFillColorVertexAttribute},									\
	AttributeInfo{2, gfx::AttributeDataType::Float2, idFillOpacityVertexAttribute},									\
};																													\
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::x, gfx::Backend::Type::Vulkan>::textures = {					\
	TextureInfo{0, idBackgroundImageTexture}																		\
};																													\

CREATE_TEST_SHADER_(CircleShader)
CREATE_TEST_SHADER_(BackgroundShader)
CREATE_TEST_SHADER_(BackgroundPatternShader)
CREATE_TEST_SHADER_(CollisionBoxShader)
CREATE_TEST_SHADER_(CollisionCircleShader)
CREATE_TEST_SHADER_(CustomSymbolIconShader)
CREATE_TEST_SHADER_(DebugShader)
CREATE_TEST_SHADER_(FillPatternShader)
CREATE_TEST_SHADER_(FillOutlinePatternShader)
CREATE_TEST_SHADER_(FillOutlineTriangulatedShader)
CREATE_TEST_SHADER_(FillExtrusionShader)
CREATE_TEST_SHADER_(FillExtrusionPatternShader)
CREATE_TEST_SHADER_(HeatmapShader)
CREATE_TEST_SHADER_(HeatmapTextureShader)
CREATE_TEST_SHADER_(HillshadeShader)
CREATE_TEST_SHADER_(HillshadePrepareShader)
//CREATE_TEST_SHADER_(LineShader)
CREATE_TEST_SHADER_(LineGradientShader)
CREATE_TEST_SHADER_(LineSDFShader)
CREATE_TEST_SHADER_(LinePatternShader)
CREATE_TEST_SHADER_(SymbolIconShader)
CREATE_TEST_SHADER_(SymbolSDFIconShader)
CREATE_TEST_SHADER_(SymbolTextAndIconShader)
CREATE_TEST_SHADER_(WideVectorShader)
CREATE_TEST_SHADER_(RasterShader)

} // namespace shaders
} // namespace mbgl
