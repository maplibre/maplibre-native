#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr const char* vertexTestShaderStr = R"(
    layout(location = 0) in vec2 position;

    void main() {
        gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
    }
)";

constexpr const char* fragmentTestShaderStr =
    R"(layout(location = 0) out vec4 outColor; void main() { outColor = vec4(1.0); })";

#define CREATE_TEST_SHADER(x)                                               \
    template <>                                                             \
    struct ShaderSource<BuiltIn::x, gfx::Backend::Type::Vulkan> {           \
        static constexpr const char* name = #x;                             \
                                                                            \
        static constexpr std::array<UniformBlockInfo, 0> uniforms{};        \
        static const std::array<AttributeInfo, 1> attributes;               \
        static constexpr std::array<AttributeInfo, 0> instanceAttributes{}; \
        static constexpr std::array<TextureInfo, 0> textures{};             \
                                                                            \
        static constexpr auto vertex = vertexTestShaderStr;                 \
        static constexpr auto fragment = fragmentTestShaderStr;             \
    };

CREATE_TEST_SHADER(CircleShader)
CREATE_TEST_SHADER(BackgroundShader)
CREATE_TEST_SHADER(BackgroundPatternShader)
CREATE_TEST_SHADER(CollisionBoxShader)
CREATE_TEST_SHADER(CollisionCircleShader)
CREATE_TEST_SHADER(CustomSymbolIconShader)
CREATE_TEST_SHADER(DebugShader)
CREATE_TEST_SHADER(HeatmapShader)
CREATE_TEST_SHADER(HeatmapTextureShader)
CREATE_TEST_SHADER(HillshadeShader)
CREATE_TEST_SHADER(HillshadePrepareShader)
CREATE_TEST_SHADER(SymbolIconShader)
CREATE_TEST_SHADER(SymbolSDFIconShader)
CREATE_TEST_SHADER(SymbolTextAndIconShader)
CREATE_TEST_SHADER(WideVectorShader)
CREATE_TEST_SHADER(RasterShader)

} // namespace shaders
} // namespace mbgl
