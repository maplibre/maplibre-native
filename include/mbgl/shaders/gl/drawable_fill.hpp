// Generated code, do not modify this file!
// Generated on 2023-05-12T17:51:34.322Z by mwilsnd using shaders/generate_shader_code.js

#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "FillShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
layout (std140) uniform DrawableUBO {
    mat4 u_matrix;
};

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform BackgroundLayerUBO {
    vec4 u_color;
    float u_opacity;
};

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
