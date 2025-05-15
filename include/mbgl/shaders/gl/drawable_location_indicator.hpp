// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "LocationIndicatorShader";
    static constexpr const char* vertex = R"(layout (std140) uniform LocationIndicatorDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
};

layout(location = 0) in vec2 a_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0.0, 1.0);
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform LocationIndicatorDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
};

void main() {
    fragColor = u_color;
}
)";
};

} // namespace shaders
} // namespace mbgl
