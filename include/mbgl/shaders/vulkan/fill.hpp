#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillShader";

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

    layout(location = 0) in vec2 inPosition;

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 1) in vec4 inColor;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 2) in vec2 inOpacity;
#endif

    layout(set = 0, binding = 1) uniform FillDrawableUBO {
        mat4 matrix;
    } drawable;

    layout(set = 0, binding = 3) uniform FillInterpolateUBO {
        float color_t;
        float opacity_t;
    } interp;

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 0) out vec4 fragColor;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 1) out float fragOpacity;
#endif

    void main() {

#if !defined(HAS_UNIFORM_u_color)
        fragColor = vec4(unpack_mix_color(inColor, interp.color_t));
#endif

#if !defined(HAS_UNIFORM_u_opacity)
        fragOpacity = unpack_mix_vec2(inOpacity, interp.opacity_t);
#endif

        gl_Position = drawable.matrix * vec4(inPosition, 0.0, 1.0);
        gl_Position.y *= -1.0;
    }
)";

    static constexpr auto fragment = R"(

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 0) in vec4 fragColor;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 1) in float fragOpacity;
#endif

    layout(location = 0) out vec4 outColor;

    layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
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
        const float opacity = fragOpacity;
#endif

        outColor = color * opacity;
    }
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillOutlineShader";

    static const std::array<UniformBlockInfo, 4> uniforms;
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

    layout(location = 0) in vec2 inPosition;

#if !defined(HAS_UNIFORM_u_outline_color)
    layout(location = 1) in vec4 inColor;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 2) in vec2 inOpacity;
#endif

    layout(set = 0, binding = 1) uniform FillDrawableUBO {
        mat4 matrix;
    } drawable;

    layout(set = 0, binding = 3) uniform FillInterpolateUBO {
        float color_t;
        float opacity_t;
    } interp;

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 0) out vec4 fragColor;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 1) out float fragOpacity;
#endif

    layout(location = 2) out vec2 fragPosition;

    void main() {

#if !defined(HAS_UNIFORM_u_color)
        fragColor = vec4(unpack_mix_color(inColor, interp.color_t));
#endif

#if !defined(HAS_UNIFORM_u_opacity)
        fragOpacity = unpack_mix_vec2(inOpacity, interp.opacity_t);
#endif

        gl_Position = drawable.matrix * vec4(inPosition, 0.0, 1.0);
        gl_Position.y *= -1.0;

        fragPosition = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * global.world_size;
    }
)";

    static constexpr auto fragment = R"(

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 0) in vec4 fragColor;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 1) in float fragOpacity;
#endif

    layout(location = 2) in float fragPosition;

    layout(location = 0) out vec4 outColor;

    layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
        vec4 color;
        vec4 outline_color;
        float opacity;
        float fade;
        float from_scale;
        float to_scale;
    } props;

    void main() {

#if defined(OVERDRAW_INSPECTOR)
    return vec4(1.0);
#endif

#if defined(HAS_UNIFORM_u_color)
        const vec4 color = props.outline_color;
#else
        const vec4 color = fragColor;
#endif

#if defined(HAS_UNIFORM_u_opacity)
        const float opacity = props.opacity;
#else
        const float opacity = fragOpacity;
#endif

        outColor = color * opacity;
    }
)";
};

} // namespace shaders
} // namespace mbgl
