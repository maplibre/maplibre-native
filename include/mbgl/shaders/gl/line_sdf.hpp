// Generated code, do not modify this file!
// Generated on 2023-04-05T16:25:15.886Z by mwilsnd using shaders/generate_shader_code.js

#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LineSDFProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* vertex = R"(// floor(127 / 2) == 63.0
// the maximum allowed miter limit is 2.0 at the moment. the extrude normal is
// stored in a byte (-128..127). we scale regular normals up to length 63, but
// there are also "special" normals that have a bigger length (of up to 126 in
// this case).
// #define scale 63.0
#define scale 0.015873016

// We scale the distance before adding it to the buffers so that we can store
// long distances for long segments. Use this value to unscale the distance.
#define LINE_DISTANCE_SCALE 2.0

layout (location = 0) in vec2 a_pos_normal;
layout (location = 1) in vec4 a_data;

uniform mat4 u_matrix;
uniform mediump float u_ratio;
uniform lowp float u_device_pixel_ratio;
uniform vec2 u_patternscale_a;
uniform float u_tex_y_a;
uniform vec2 u_patternscale_b;
uniform float u_tex_y_b;
uniform vec2 u_units_to_pixels;

out vec2 v_normal;
out vec2 v_width2;
out vec2 v_tex_a;
out vec2 v_tex_b;
out float v_gamma_scale;

#ifndef HAS_UNIFORM_u_color
uniform lowp float u_color_t;
layout (location = 2) in highp vec4 a_color;
out highp vec4 color;
#else
uniform highp vec4 u_color;
#endif
#ifndef HAS_UNIFORM_u_blur
uniform lowp float u_blur_t;
layout (location = 3) in lowp vec2 a_blur;
out lowp float blur;
#else
uniform lowp float u_blur;
#endif
#ifndef HAS_UNIFORM_u_opacity
uniform lowp float u_opacity_t;
layout (location = 4) in lowp vec2 a_opacity;
out lowp float opacity;
#else
uniform lowp float u_opacity;
#endif
#ifndef HAS_UNIFORM_u_gapwidth
uniform lowp float u_gapwidth_t;
layout (location = 5) in mediump vec2 a_gapwidth;
#else
uniform mediump float u_gapwidth;
#endif
#ifndef HAS_UNIFORM_u_offset
uniform lowp float u_offset_t;
layout (location = 6) in lowp vec2 a_offset;
#else
uniform lowp float u_offset;
#endif
#ifndef HAS_UNIFORM_u_width
uniform lowp float u_width_t;
layout (location = 7) in mediump vec2 a_width;
out mediump float width;
#else
uniform mediump float u_width;
#endif
#ifndef HAS_UNIFORM_u_floorwidth
uniform lowp float u_floorwidth_t;
layout (location = 8) in lowp vec2 a_floorwidth;
out lowp float floorwidth;
#else
uniform lowp float u_floorwidth;
#endif

void main() {
    #ifndef HAS_UNIFORM_u_color
color = unpack_mix_color(a_color, u_color_t);
#else
highp vec4 color = u_color;
#endif
    #ifndef HAS_UNIFORM_u_blur
blur = unpack_mix_vec2(a_blur, u_blur_t);
#else
lowp float blur = u_blur;
#endif
    #ifndef HAS_UNIFORM_u_opacity
opacity = unpack_mix_vec2(a_opacity, u_opacity_t);
#else
lowp float opacity = u_opacity;
#endif
    #ifndef HAS_UNIFORM_u_gapwidth
mediump float gapwidth = unpack_mix_vec2(a_gapwidth, u_gapwidth_t);
#else
mediump float gapwidth = u_gapwidth;
#endif
    #ifndef HAS_UNIFORM_u_offset
lowp float offset = unpack_mix_vec2(a_offset, u_offset_t);
#else
lowp float offset = u_offset;
#endif
    #ifndef HAS_UNIFORM_u_width
width = unpack_mix_vec2(a_width, u_width_t);
#else
mediump float width = u_width;
#endif
    #ifndef HAS_UNIFORM_u_floorwidth
floorwidth = unpack_mix_vec2(a_floorwidth, u_floorwidth_t);
#else
lowp float floorwidth = u_floorwidth;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    float ANTIALIASING = 1.0 / u_device_pixel_ratio / 2.0;

    vec2 a_extrude = a_data.xy - 128.0;
    float a_direction = mod(a_data.z, 4.0) - 1.0;
    float a_linesofar = (floor(a_data.z / 4.0) + a_data.w * 64.0) * LINE_DISTANCE_SCALE;

    vec2 pos = floor(a_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of a_pos_normal
    mediump vec2 normal = a_pos_normal - 2.0 * pos;
    normal.y = normal.y * 2.0 - 1.0;
    v_normal = normal;

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    gapwidth = gapwidth / 2.0;
    float halfwidth = width / 2.0;
    offset = -1.0 * offset;

    float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width
    // of this vertex.
    mediump vec2 dist =outset * a_extrude * scale;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    mediump float u = 0.5 * a_direction;
    mediump float t = 1.0 - abs(u);
    mediump vec2 offset2 = offset * a_extrude * scale * normal.y * mat2(t, -u, u, t);

    vec4 projected_extrude = u_matrix * vec4(dist / u_ratio, 0.0, 0.0);
    gl_Position = u_matrix * vec4(pos + offset2 / u_ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * u_units_to_pixels);
    v_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

    v_tex_a = vec2(a_linesofar * u_patternscale_a.x / floorwidth, normal.y * u_patternscale_a.y + u_tex_y_a);
    v_tex_b = vec2(a_linesofar * u_patternscale_b.x / floorwidth, normal.y * u_patternscale_b.y + u_tex_y_b);

    v_width2 = vec2(outset, inset);
}
)";
    static constexpr const char* fragment = R"(
uniform lowp float u_device_pixel_ratio;
uniform sampler2D u_image;
uniform float u_sdfgamma;
uniform float u_mix;

in vec2 v_normal;
in vec2 v_width2;
in vec2 v_tex_a;
in vec2 v_tex_b;
in float v_gamma_scale;

#ifndef HAS_UNIFORM_u_color
in highp vec4 color;
#else
uniform highp vec4 u_color;
#endif
#ifndef HAS_UNIFORM_u_blur
in lowp float blur;
#else
uniform lowp float u_blur;
#endif
#ifndef HAS_UNIFORM_u_opacity
in lowp float opacity;
#else
uniform lowp float u_opacity;
#endif
#ifndef HAS_UNIFORM_u_width
in mediump float width;
#else
uniform mediump float u_width;
#endif
#ifndef HAS_UNIFORM_u_floorwidth
in lowp float floorwidth;
#else
uniform lowp float u_floorwidth;
#endif

void main() {
    #ifdef HAS_UNIFORM_u_color
highp vec4 color = u_color;
#endif
    #ifdef HAS_UNIFORM_u_blur
lowp float blur = u_blur;
#endif
    #ifdef HAS_UNIFORM_u_opacity
lowp float opacity = u_opacity;
#endif
    #ifdef HAS_UNIFORM_u_width
mediump float width = u_width;
#endif
    #ifdef HAS_UNIFORM_u_floorwidth
lowp float floorwidth = u_floorwidth;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    float dist = length(v_normal) * v_width2.s;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (v_width2.t) or when fading out
    // (v_width2.s)
    float blur2 = (blur + 1.0 / u_device_pixel_ratio) * v_gamma_scale;
    float alpha = clamp(min(dist - (v_width2.t - blur2), v_width2.s - dist) / blur2, 0.0, 1.0);

    float sdfdist_a = texture(u_image, v_tex_a).a;
    float sdfdist_b = texture(u_image, v_tex_b).a;
    float sdfdist = mix(sdfdist_a, sdfdist_b, u_mix);
    alpha *= smoothstep(0.5 - u_sdfgamma / floorwidth, 0.5 + u_sdfgamma / floorwidth, sdfdist);

    fragColor = color * (alpha * opacity);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
