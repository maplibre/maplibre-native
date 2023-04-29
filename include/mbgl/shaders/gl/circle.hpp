// Generated code, do not modify this file!
// Generated on 2023-04-05T16:25:15.886Z by mwilsnd using shaders/generate_shader_code.js

#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CircleProgram, gfx::Backend::Type::OpenGL> {
    static constexpr const char* vertex = R"(uniform mat4 u_matrix;
uniform bool u_scale_with_map;
uniform bool u_pitch_with_map;
uniform vec2 u_extrude_scale;
uniform lowp float u_device_pixel_ratio;
uniform highp float u_camera_to_center_distance;

layout (location = 0) in vec2 a_pos;
out vec3 v_data;

#ifndef HAS_UNIFORM_u_color
uniform lowp float u_color_t;
layout (location = 1) in highp vec4 a_color;
out highp vec4 color;
#else
uniform highp vec4 u_color;
#endif
#ifndef HAS_UNIFORM_u_radius
uniform lowp float u_radius_t;
layout (location = 2) in mediump vec2 a_radius;
out mediump float radius;
#else
uniform mediump float u_radius;
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
#ifndef HAS_UNIFORM_u_stroke_color
uniform lowp float u_stroke_color_t;
layout (location = 5) in highp vec4 a_stroke_color;
out highp vec4 stroke_color;
#else
uniform highp vec4 u_stroke_color;
#endif
#ifndef HAS_UNIFORM_u_stroke_width
uniform lowp float u_stroke_width_t;
layout (location = 6) in mediump vec2 a_stroke_width;
out mediump float stroke_width;
#else
uniform mediump float u_stroke_width;
#endif
#ifndef HAS_UNIFORM_u_stroke_opacity
uniform lowp float u_stroke_opacity_t;
layout (location = 7) in lowp vec2 a_stroke_opacity;
out lowp float stroke_opacity;
#else
uniform lowp float u_stroke_opacity;
#endif

void main(void) {
    #ifndef HAS_UNIFORM_u_color
color = unpack_mix_color(a_color, u_color_t);
#else
highp vec4 color = u_color;
#endif
    #ifndef HAS_UNIFORM_u_radius
radius = unpack_mix_vec2(a_radius, u_radius_t);
#else
mediump float radius = u_radius;
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
    #ifndef HAS_UNIFORM_u_stroke_color
stroke_color = unpack_mix_color(a_stroke_color, u_stroke_color_t);
#else
highp vec4 stroke_color = u_stroke_color;
#endif
    #ifndef HAS_UNIFORM_u_stroke_width
stroke_width = unpack_mix_vec2(a_stroke_width, u_stroke_width_t);
#else
mediump float stroke_width = u_stroke_width;
#endif
    #ifndef HAS_UNIFORM_u_stroke_opacity
stroke_opacity = unpack_mix_vec2(a_stroke_opacity, u_stroke_opacity_t);
#else
lowp float stroke_opacity = u_stroke_opacity;
#endif

    // unencode the extrusion vector that we snuck into the a_pos vector
    vec2 extrude = vec2(mod(a_pos, 2.0) * 2.0 - 1.0);

    // multiply a_pos by 0.5, since we had it * 2 in order to sneak
    // in extrusion data
    vec2 circle_center = floor(a_pos * 0.5);
    if (u_pitch_with_map) {
        vec2 corner_position = circle_center;
        if (u_scale_with_map) {
            corner_position += extrude * (radius + stroke_width) * u_extrude_scale;
        } else {
            // Pitching the circle with the map effectively scales it with the map
            // To counteract the effect for pitch-scale: viewport, we rescale the
            // whole circle based on the pitch scaling effect at its central point
            vec4 projected_center = u_matrix * vec4(circle_center, 0, 1);
            corner_position += extrude * (radius + stroke_width) * u_extrude_scale * (projected_center.w / u_camera_to_center_distance);
        }

        gl_Position = u_matrix * vec4(corner_position, 0, 1);
    } else {
        gl_Position = u_matrix * vec4(circle_center, 0, 1);

        if (u_scale_with_map) {
            gl_Position.xy += extrude * (radius + stroke_width) * u_extrude_scale * u_camera_to_center_distance;
        } else {
            gl_Position.xy += extrude * (radius + stroke_width) * u_extrude_scale * gl_Position.w;
        }
    }

    // This is a minimum blur distance that serves as a faux-antialiasing for
    // the circle. since blur is a ratio of the circle's size and the intent is
    // to keep the blur at roughly 1px, the two are inversely related.
    lowp float antialiasblur = 1.0 / u_device_pixel_ratio / (radius + stroke_width);

    v_data = vec3(extrude.x, extrude.y, antialiasblur);
}
)";
    static constexpr const char* fragment = R"(in vec3 v_data;

#ifndef HAS_UNIFORM_u_color
in highp vec4 color;
#else
uniform highp vec4 u_color;
#endif
#ifndef HAS_UNIFORM_u_radius
in mediump float radius;
#else
uniform mediump float u_radius;
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
#ifndef HAS_UNIFORM_u_stroke_color
in highp vec4 stroke_color;
#else
uniform highp vec4 u_stroke_color;
#endif
#ifndef HAS_UNIFORM_u_stroke_width
in mediump float stroke_width;
#else
uniform mediump float u_stroke_width;
#endif
#ifndef HAS_UNIFORM_u_stroke_opacity
in lowp float stroke_opacity;
#else
uniform lowp float u_stroke_opacity;
#endif

void main() {
    #ifdef HAS_UNIFORM_u_color
highp vec4 color = u_color;
#endif
    #ifdef HAS_UNIFORM_u_radius
mediump float radius = u_radius;
#endif
    #ifdef HAS_UNIFORM_u_blur
lowp float blur = u_blur;
#endif
    #ifdef HAS_UNIFORM_u_opacity
lowp float opacity = u_opacity;
#endif
    #ifdef HAS_UNIFORM_u_stroke_color
highp vec4 stroke_color = u_stroke_color;
#endif
    #ifdef HAS_UNIFORM_u_stroke_width
mediump float stroke_width = u_stroke_width;
#endif
    #ifdef HAS_UNIFORM_u_stroke_opacity
lowp float stroke_opacity = u_stroke_opacity;
#endif

    vec2 extrude = v_data.xy;
    float extrude_length = length(extrude);

    lowp float antialiasblur = v_data.z;
    float antialiased_blur = -max(blur, antialiasblur);

    float opacity_t = smoothstep(0.0, antialiased_blur, extrude_length - 1.0);

    float color_t = stroke_width < 0.01 ? 0.0 : smoothstep(
        antialiased_blur,
        0.0,
        extrude_length - radius / (radius + stroke_width)
    );

    fragColor = opacity_t * mix(color * opacity, stroke_color * stroke_opacity, color_t);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
