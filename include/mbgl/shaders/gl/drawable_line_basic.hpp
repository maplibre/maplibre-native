// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LineBasicShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "LineBasicShader";
    static constexpr const char* vertex = R"(// floor(127 / 2) == 63.0
// the maximum allowed miter limit is 2.0 at the moment. the extrude normal is
// stored in a byte (-128..127). we scale regular normals up to length 63, but
// there are also "special" normals that have a bigger length (of up to 126 in
// this case).
// #define scale 63.0
#define scale 0.015873016

layout (location = 0) in vec2 a_pos_normal;
layout (location = 1) in vec4 a_data;

layout (std140) uniform LineBasicUBO {
    highp mat4 u_matrix;
    highp vec2 u_units_to_pixels;
    mediump float u_ratio;
    lowp float pad0;
};

layout (std140) uniform LineBasicPropertiesUBO {
    highp vec4 u_color;
    lowp float u_opacity;
    mediump float u_width;

    highp vec2 pad1;
};

out vec2 v_normal;
out float v_width;
out float v_gamma_scale;
out highp float v_linesofar;

void main() {
    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    vec2 a_extrude = a_data.xy - 128.0;

    v_linesofar = (floor(a_data.z / 4.0) + a_data.w * 64.0) * 2.0;

    vec2 pos = floor(a_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of a_pos_normal
    mediump vec2 normal = a_pos_normal - 2.0 * pos;
    normal.y = normal.y * 2.0 - 1.0;
    v_normal = normal;

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = u_width / 2.0;
    float outset = halfwidth + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width
    // of this vertex.
    mediump vec2 dist = outset * a_extrude * scale;

    vec4 projected_extrude = u_matrix * vec4(dist / u_ratio, 0.0, 0.0);
    gl_Position = u_matrix * vec4(pos, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * u_units_to_pixels);
    v_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

    v_width = outset;
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform LineBasicUBO {
    highp mat4 u_matrix;
    highp vec2 u_units_to_pixels;
    mediump float u_ratio;
    lowp float pad0;
};

layout (std140) uniform LineBasicPropertiesUBO {
    highp vec4 u_color;
    lowp float u_opacity;
    mediump float u_width;

    highp vec2 pad1;
};

in float v_width;
in vec2 v_normal;
in float v_gamma_scale;

void main() {
    // Calculate the distance of the pixel from the line in pixels.
    float dist = length(v_normal) * v_width;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (v_width2.t) or when fading out
    // (v_width2.s)
    float blur2 = (1.0 / DEVICE_PIXEL_RATIO) * v_gamma_scale;
    float alpha = clamp(min(dist + blur2, v_width - dist) / blur2, 0.0, 1.0);

    fragColor = u_color * (alpha * u_opacity);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
