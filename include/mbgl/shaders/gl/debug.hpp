// Generated code, do not modify this file!
// Generated on 2023-04-04T01:24:40.539Z by mwilsnd using shaders/generate_shader_code.js

#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::DebugProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* vertex = R"(attribute vec2 a_pos;
varying vec2 v_uv;

uniform mat4 u_matrix;
uniform float u_overlay_scale;

void main() {
    // This vertex shader expects a EXTENT x EXTENT quad,
    // The UV co-ordinates for the overlay texture can be calculated using that knowledge
    v_uv = a_pos / 8192.0;
    gl_Position = u_matrix * vec4(a_pos * u_overlay_scale, 0, 1);
}
)";
    static constexpr const char* fragment = R"(uniform highp vec4 u_color;
uniform sampler2D u_overlay;

varying vec2 v_uv;

void main() {
    vec4 overlay_color = texture2D(u_overlay, v_uv);
    gl_FragColor = mix(u_color, overlay_color, overlay_color.a);
}
)";
};

} // namespace shaders
} // namespace mbgl
