// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>


namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::DebugProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "DebugProgram";


    static constexpr const char* vertexData = R"(layout (location = 0) in vec2 a_pos;
out vec2 v_uv;

uniform mat4 u_matrix;
uniform float u_overlay_scale;

void main() {
    // This vertex shader expects a EXTENT x EXTENT quad,
    // The UV co-ordinates for the overlay texture can be calculated using that knowledge
    v_uv = a_pos / 8192.0;
    gl_Position = u_matrix * vec4(a_pos * u_overlay_scale, 0, 1);
}
)";
    static constexpr const char* fragmentData = R"(uniform highp vec4 u_color;
uniform sampler2D u_overlay;

in vec2 v_uv;

void main() {
    vec4 overlay_color = texture(u_overlay, v_uv);
    fragColor = mix(u_color, overlay_color, overlay_color.a);
}
)";
    static std::string vertex() {
        using Ty = ShaderSource<BuiltIn::DebugProgram, gfx::Backend::Type::OpenGL>;
        return Ty::vertexData;
    }
    static std::string fragment() {
        using Ty = ShaderSource<BuiltIn::DebugProgram, gfx::Backend::Type::OpenGL>;
        return Ty::fragmentData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
