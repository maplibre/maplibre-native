// Generated code, do not modify this file!
// Generated on 2023-04-04T01:24:40.539Z by mwilsnd using shaders/generate_shader_code.js

#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HeatmapTextureProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* vertex = R"(uniform mat4 u_matrix;
uniform vec2 u_world;
attribute vec2 a_pos;
varying vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos * u_world, 0, 1);

    v_pos.x = a_pos.x;
    v_pos.y = 1.0 - a_pos.y;
}
)";
    static constexpr const char* fragment = R"(uniform sampler2D u_image;
uniform sampler2D u_color_ramp;
uniform float u_opacity;
varying vec2 v_pos;

void main() {
    float t = texture2D(u_image, v_pos).r;
    vec4 color = texture2D(u_color_ramp, vec2(t, 0.5));
    gl_FragColor = color * u_opacity;

#ifdef OVERDRAW_INSPECTOR
    gl_FragColor = vec4(0.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
