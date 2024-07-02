#include <mbgl/shaders/vulkan/circle.hpp>

namespace mbgl {
namespace shaders {

#define CREATE_TEST_SHADER_(x)                                                                                      \
const std::array<AttributeInfo, 1> ShaderSource<BuiltIn::x, gfx::Backend::Type::Vulkan>::attributes = {				\
	AttributeInfo{0, gfx::AttributeDataType::Short2, idFillPosVertexAttribute},										\
};																													\

CREATE_TEST_SHADER_(CircleShader)
CREATE_TEST_SHADER_(BackgroundShader)
CREATE_TEST_SHADER_(BackgroundPatternShader)
CREATE_TEST_SHADER_(CollisionBoxShader)
CREATE_TEST_SHADER_(CollisionCircleShader)
CREATE_TEST_SHADER_(CustomSymbolIconShader)
CREATE_TEST_SHADER_(DebugShader)
CREATE_TEST_SHADER_(HeatmapShader)
CREATE_TEST_SHADER_(HeatmapTextureShader)
CREATE_TEST_SHADER_(HillshadeShader)
CREATE_TEST_SHADER_(HillshadePrepareShader)
CREATE_TEST_SHADER_(SymbolIconShader)
CREATE_TEST_SHADER_(SymbolSDFIconShader)
CREATE_TEST_SHADER_(SymbolTextAndIconShader)
CREATE_TEST_SHADER_(WideVectorShader)
CREATE_TEST_SHADER_(RasterShader)

} // namespace shaders
} // namespace mbgl
