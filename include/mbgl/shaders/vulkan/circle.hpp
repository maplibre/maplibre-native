#pragma once

#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr const char* vertexTestShaderStr = R"(
    layout(location = 0) in vec2 position;

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 1) in vec4 color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 2) in vec2 opacity;
#endif

    layout(set = 1, binding = 0) uniform FillDrawableUBO {
        mat4 matrix;
    } drawable;

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 0) out vec4 fragColor;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 1) out vec2 fragOpacity;
#endif

    void main() {

#if !defined(HAS_UNIFORM_u_color)
        fragColor = color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
        fragOpacity = opacity;
#endif

        gl_Position = drawable.matrix * vec4(position, 0.0, 1.0);
        gl_Position.y *= -1.0;
    }
)";

constexpr const char* fragmentTestShaderStr = R"(
#if !defined(HAS_UNIFORM_u_color)
    layout(location = 0) in vec4 fragColor;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 1) in vec2 fragOpacity;
#endif

    layout(location = 0) out vec4 outColor;

    layout(set = 4, binding = 0) uniform FillEvaluatedPropsUBO {
        vec4 color;
        vec4 outline_color;
        float opacity;
        float fade;
        float from_scale;
        float to_scale;
    } props;

    void main() {

#if defined(HAS_UNIFORM_u_color)
        const vec4 color = props.color;
#else
        const vec4 color = fragColor;
#endif

#if defined(HAS_UNIFORM_u_opacity)
        const float opacity = props.opacity;
#else
        const float opacity = fragOpacity.x;
#endif

        outColor = color * opacity;
    }
)";

#define CREATE_TEST_SHADER(x)                                                            \
template <>                                                                              \
struct ShaderSource<BuiltIn::x, gfx::Backend::Type::Vulkan>  {                           \
    static constexpr const char* name = #x;                                              \
                                                                                         \
    static const std::array<UniformBlockInfo, 2> uniforms;                               \
    static const std::array<AttributeInfo, 3> attributes;                                \
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};                  \
    static const std::array<TextureInfo, 0> textures;                                    \
                                                                                         \
    static constexpr auto vertex = vertexTestShaderStr;                                  \
    static constexpr auto fragment = fragmentTestShaderStr;                              \
};

CREATE_TEST_SHADER(CircleShader)
CREATE_TEST_SHADER(BackgroundShader)
CREATE_TEST_SHADER(BackgroundPatternShader)
CREATE_TEST_SHADER(ClippingMaskProgram)
CREATE_TEST_SHADER(CollisionBoxShader)
CREATE_TEST_SHADER(CollisionCircleShader)
CREATE_TEST_SHADER(CustomSymbolIconShader)
CREATE_TEST_SHADER(DebugShader)
CREATE_TEST_SHADER(FillPatternShader)
CREATE_TEST_SHADER(FillOutlinePatternShader)
CREATE_TEST_SHADER(FillOutlineTriangulatedShader)
CREATE_TEST_SHADER(FillExtrusionShader)
CREATE_TEST_SHADER(FillExtrusionPatternShader)
CREATE_TEST_SHADER(HeatmapShader)
CREATE_TEST_SHADER(HeatmapTextureShader)
CREATE_TEST_SHADER(HillshadeShader)
CREATE_TEST_SHADER(HillshadePrepareShader)
CREATE_TEST_SHADER(LineShader)
CREATE_TEST_SHADER(LineGradientShader)
CREATE_TEST_SHADER(LineSDFShader)
CREATE_TEST_SHADER(LinePatternShader)
CREATE_TEST_SHADER(SymbolIconShader)
CREATE_TEST_SHADER(SymbolSDFIconShader)
CREATE_TEST_SHADER(SymbolTextAndIconShader)
CREATE_TEST_SHADER(WideVectorShader)
CREATE_TEST_SHADER(RasterShader)

} // namespace shaders
} // namespace mbgl
