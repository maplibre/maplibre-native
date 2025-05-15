// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CustomGeometryShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "CustomGeometryShader";
    static constexpr const char* vertex = R"(layout (std140) uniform CustomGeometryDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
};

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 frag_uv;

void main() {
    frag_uv = a_uv;
    gl_Position = u_matrix * vec4(a_pos, 1.0);
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform CustomGeometryDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
};

in vec2 frag_uv;
uniform sampler2D u_image;

void main() {
    fragColor = texture(u_image, frag_uv) * u_color;
}
)";
};

} // namespace shaders
} // namespace mbgl
