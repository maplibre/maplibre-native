// Generated code, do not modify this file!
// Generated on 2023-04-05T16:25:15.886Z by mwilsnd using shaders/generate_shader_code.js

#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillOutlineProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;

uniform mat4 u_matrix;
uniform vec2 u_world;

out vec2 v_pos;

#ifndef HAS_UNIFORM_u_outline_color
uniform lowp float u_outline_color_t;
layout (location = 1) in highp vec4 a_outline_color;
out highp vec4 outline_color;
#else
uniform highp vec4 u_outline_color;
#endif
#ifndef HAS_UNIFORM_u_opacity
uniform lowp float u_opacity_t;
layout (location = 2) in lowp vec2 a_opacity;
out lowp float opacity;
#else
uniform lowp float u_opacity;
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
    static constexpr const char* fragment = R"(in vec2 v_pos;

#ifndef HAS_UNIFORM_u_outline_color
in highp vec4 outline_color;
#else
uniform highp vec4 u_outline_color;
#endif
#ifndef HAS_UNIFORM_u_opacity
in lowp float opacity;
#else
uniform lowp float u_opacity;
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
