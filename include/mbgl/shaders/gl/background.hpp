// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "BackgroundProgram";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
uniform mat4 u_matrix;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
)";
    static constexpr const char* fragment = R"(uniform vec4 u_color;
uniform float u_opacity;

void main() {
    fragColor = u_color * u_opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
