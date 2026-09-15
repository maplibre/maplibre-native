// Generated code, do not modify this file!
#pragma once
#include <mln/shaders/shader_source.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::SkyShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "SkyShader";
    static constexpr const char* vertex = R"(layout(location = 0) in vec2 a_pos;

out vec2 v_pos;

void main() {
    v_pos = a_pos;
    gl_Position = vec4(a_pos, 1.0, 1.0);
}
)";
    static constexpr const char* fragment = R"(in vec2 v_pos;

layout(std140) uniform SkyPropsUBO {
    vec4 sky_color;
    vec4 horizon_color;
    vec2 horizon;
    vec2 horizon_normal;
    vec2 viewport_size;
    float sky_horizon_blend;
    float sky_blend;
} sky;

void main() {
    vec2 pixel = (v_pos * 0.5 + 0.5) * sky.viewport_size;
    float distance_to_horizon = dot(pixel - sky.horizon, sky.horizon_normal);
    vec4 color = vec4(0.0);

    if (distance_to_horizon > 0.0) {
        if (sky.sky_horizon_blend > 0.0 && distance_to_horizon < sky.sky_horizon_blend) {
            float blend = 1.0 - distance_to_horizon / sky.sky_horizon_blend;
            color = mix(sky.sky_color, sky.horizon_color, blend * blend);
        } else {
            color = sky.sky_color;
        }
    }

    fragColor = color * (1.0 - sky.sky_blend);
}
)";
};

} // namespace shaders
} // namespace mln
