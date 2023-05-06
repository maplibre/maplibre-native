// Generated code, do not modify this file!
// Generated on 2023-04-04T01:24:40.539Z by mwilsnd using shaders/generate_shader_code.js

#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* vertex = R"(attribute vec2 a_pos;

uniform mat4 u_matrix;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
)";
    static constexpr const char* fragment = R"(void main() {
    gl_FragColor = vec4(1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl
