// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "CustomSymbolIconShader";
    static constexpr const char* vertex = R"(layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex;

layout(std140) uniform CustomSymbolIconDrawableUBO {
    highp mat4 u_matrix;
};

layout(std140) uniform CustomSymbolIconParametersUBO {
    highp vec2 u_extrude_scale;
    highp vec2 u_anchor;
    highp float u_angle_degrees;
    bool u_scale_with_map;
    bool u_pitch_with_map;
    highp float u_camera_to_center_distance;
};

out vec2 v_tex;

vec2 rotateVec2(vec2 v, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    return vec2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

void main() {
    // unencode the extrusion vector (-1, -1) to (1, 1)
    vec2 extrude = vec2(mod(a_pos, 2.0) * 2.0 - 1.0);

    // anchor in the range (-1, -1) to (1, 1)
    vec2 anchor = (u_anchor - vec2(0.5, 0.5)) * 2.0;

    // get center
    vec2 center = floor(a_pos * 0.5);
    float angle = radians(-u_angle_degrees);
    vec2 rotated_unit = rotateVec2((extrude - anchor), angle);

    // compute
    if (u_pitch_with_map) {
        vec2 corner = center;
        if (u_scale_with_map) {
            corner += rotated_unit * u_extrude_scale;
        } else {
            vec4 projected_center = u_matrix * vec4(center, 0, 1);
            corner += rotated_unit * u_extrude_scale * (projected_center.w / u_camera_to_center_distance);
        }

        gl_Position = u_matrix * vec4(corner, 0, 1);
    } else {
        gl_Position = u_matrix * vec4(center, 0, 1);

        if (u_scale_with_map) {
            gl_Position.xy += rotated_unit * u_extrude_scale * u_camera_to_center_distance;
        } else {
            gl_Position.xy += rotated_unit * u_extrude_scale * gl_Position.w;
        }
    }

    // texture coordinates
    v_tex = a_tex;
}
)";
    static constexpr const char* fragment = R"(uniform sampler2D u_texture;

in vec2 v_tex;

void main() {
    fragColor = texture(u_texture, v_tex);
}
)";
};

} // namespace shaders
} // namespace mbgl
