// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "ClippingMaskProgram";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;

uniform mat4 u_matrix;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
)";
    static constexpr const char* fragment = R"(void main() {
    fragColor = vec4(1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl
