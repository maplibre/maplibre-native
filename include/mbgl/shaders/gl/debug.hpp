// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "DebugShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
out vec2 v_uv;

layout (std140) uniform DebugUBO {
    highp mat4 u_matrix;
    highp vec4 u_color;
    highp float u_overlay_scale;
    lowp float pad1;
    lowp float pad2;
    lowp float pad3;
};

void main() {
    // This vertex shader expects a EXTENT x EXTENT quad,
    // The UV co-ordinates for the overlay texture can be calculated using that knowledge
    v_uv = a_pos / 8192.0;
    gl_Position = u_matrix * vec4(a_pos * u_overlay_scale, 0, 1);
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform DebugUBO {
    highp mat4 u_matrix;
    highp vec4 u_color;
    highp float u_overlay_scale;
    lowp float pad1;
    lowp float pad2;
    lowp float pad3;
};

uniform sampler2D u_overlay;

in vec2 v_uv;

void main() {
    vec4 overlay_color = texture(u_overlay, v_uv);
    fragColor = mix(u_color, overlay_color, overlay_color.a);
}
)";
};

} // namespace shaders
} // namespace mbgl
