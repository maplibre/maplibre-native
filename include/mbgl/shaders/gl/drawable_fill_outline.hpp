// Generated code, do not modify this file!
// Generated on 2023-05-25T15:19:24.210Z by timsylvester using shaders/generate_shader_code.js

#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "FillOutlineShader";
    static constexpr const char* vertex = R"(layout (std140) uniform FillDrawableUBO {
    highp mat4 u_matrix;
    highp vec4 u_scale;
    highp vec2 u_world;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp vec2 u_texsize;
    highp float u_fade;

    highp float u_color_t;
    highp float u_opacity_t;
    highp float u_outline_color_t;
    highp float u_pattern_from_t;
    highp float u_pattern_to_t;

    highp vec2 u_color;
    highp vec2 u_opacity;
    highp vec2 u_outline_color_pad;
    highp vec4 u_outline_color;
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
};

layout (location = 0) in vec2 a_pos;

out vec2 v_pos;

#ifndef HAS_UNIFORM_u_outline_color
layout (location = 1) in highp vec4 a_outline_color;
out highp vec4 outline_color;
#endif
#ifndef HAS_UNIFORM_u_opacity
layout (location = 2) in lowp vec2 a_opacity;
out lowp float opacity;
#endif

void main() {
    #ifndef HAS_UNIFORM_u_outline_color
outline_color = unpack_mix_color(a_outline_color, u_outline_color_t);
#else
highp vec4 outline_color = u_outline_color;
#endif
    #ifndef HAS_UNIFORM_u_opacity
opacity = unpack_mix_vec2(a_opacity, u_opacity_t);
#else
lowp float opacity = u_opacity;
#endif

    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    v_pos = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * u_world;
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform FillDrawableUBO {
    highp mat4 u_matrix;
    highp vec4 u_scale;
    highp vec2 u_world;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp vec2 u_texsize;
    highp float u_fade;

    highp float u_color_t;
    highp float u_opacity_t;
    highp float u_outline_color_t;
    highp float u_pattern_from_t;
    highp float u_pattern_to_t;

    highp vec2 u_color;
    highp vec2 u_opacity;
    highp vec2 u_outline_color_pad;
    highp vec4 u_outline_color;
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
};

in vec2 v_pos;

#ifndef HAS_UNIFORM_u_outline_color
in highp vec4 outline_color;
#endif
#ifndef HAS_UNIFORM_u_opacity
in lowp float opacity;
#endif

void main() {
    #ifdef HAS_UNIFORM_u_outline_color
highp vec4 outline_color = u_outline_color;
#endif
    #ifdef HAS_UNIFORM_u_opacity
lowp float opacity = u_opacity;
#endif

    float dist = length(v_pos - gl_FragCoord.xy);
    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    fragColor = outline_color * (alpha * opacity);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
