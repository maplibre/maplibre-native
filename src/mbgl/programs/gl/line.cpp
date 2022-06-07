// NOTE: DO NOT CHANGE THIS FILE. IT IS AUTOMATICALLY GENERATED.
// clang-format off
#include <mbgl/programs/line_program.hpp>
#include <mbgl/programs/gl/preludes.hpp>
#include <mbgl/programs/gl/shader_source.hpp>
#include <mbgl/gl/program.hpp>

namespace mbgl {
namespace programs {
namespace gl {

template <typename>
struct ShaderSource;

template <>
struct ShaderSource<LineProgram> {
    static constexpr const char* name = "line";
    static constexpr const uint8_t hash[8] = {0x68, 0xd0, 0x9a, 0x26, 0x34, 0x90, 0x8c, 0x92};
    static constexpr const auto vertexOffset = 35171;
    static constexpr const auto fragmentOffset = 38218;
};

constexpr const char* ShaderSource<LineProgram>::name;
constexpr const uint8_t ShaderSource<LineProgram>::hash[8];

} // namespace gl
} // namespace programs

namespace gfx {

template <>
std::unique_ptr<gfx::Program<LineProgram>>
Backend::Create<gfx::Backend::Type::OpenGL>(const ProgramParameters& programParameters) {
    return std::make_unique<gl::Program<LineProgram>>(programParameters);
}

} // namespace gfx
} // namespace mbgl

// Uncompressed source of line.vertex.glsl:
/*

#define scale 0.015873016
attribute vec2 a_pos_normal;
attribute vec4 a_data;
uniform mat4 u_matrix;
uniform mediump float u_ratio;
uniform vec2 u_units_to_pixels;
uniform lowp float u_device_pixel_ratio;
varying vec2 v_normal;
varying vec2 v_width2;
varying float v_gamma_scale;
varying highp float v_linesofar;

#ifndef HAS_UNIFORM_u_color
uniform lowp float u_color_t;
attribute highp vec4 a_color;
varying highp vec4 color;
#else
uniform highp vec4 u_color;
#endif


#ifndef HAS_UNIFORM_u_blur
uniform lowp float u_blur_t;
attribute lowp vec2 a_blur;
varying lowp float blur;
#else
uniform lowp float u_blur;
#endif


#ifndef HAS_UNIFORM_u_opacity
uniform lowp float u_opacity_t;
attribute lowp vec2 a_opacity;
varying lowp float opacity;
#else
uniform lowp float u_opacity;
#endif


#ifndef HAS_UNIFORM_u_gapwidth
uniform lowp float u_gapwidth_t;
attribute mediump vec2 a_gapwidth;
#else
uniform mediump float u_gapwidth;
#endif


#ifndef HAS_UNIFORM_u_offset
uniform lowp float u_offset_t;
attribute lowp vec2 a_offset;
#else
uniform lowp float u_offset;
#endif


#ifndef HAS_UNIFORM_u_width
uniform lowp float u_width_t;
attribute mediump vec2 a_width;
#else
uniform mediump float u_width;
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
    mediump float width = unpack_mix_vec2(a_width, u_width_t);
#else
    mediump float width = u_width;
#endif

    float ANTIALIASING = 1.0 / u_device_pixel_ratio / 2.0;
    vec2 a_extrude = a_data.xy - 128.0;
    float a_direction = mod(a_data.z, 4.0) - 1.0;
    v_linesofar = (floor(a_data.z / 4.0) + a_data.w * 64.0) * 2.0;
    vec2 pos = floor(a_pos_normal * 0.5);
    mediump vec2 normal = a_pos_normal - 2.0 * pos;
    normal.y = normal.y * 2.0 - 1.0;
    v_normal = normal;
    gapwidth = gapwidth / 2.0;
    float halfwidth = width / 2.0;
    offset = -1.0 * offset;
    float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);
    mediump vec2 dist = outset * a_extrude * scale;
    mediump float u = 0.5 * a_direction;
    mediump float t = 1.0 - abs(u);
    mediump vec2 offset2 = offset * a_extrude * scale * normal.y * mat2(t, -u, u, t);
    vec4 projected_extrude = u_matrix * vec4(dist / u_ratio, 0.0, 0.0);
    gl_Position = u_matrix * vec4(pos + offset2 / u_ratio, 0.0, 1.0) + projected_extrude;
    #ifdef TERRAIN3D
        v_gamma_scale = 1.0;
    #else
        float extrude_length_without_perspective = length(dist);
        float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * u_units_to_pixels);
        v_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;
    #endif
    v_width2 = vec2(outset, inset);
}
*/

// Uncompressed source of line.fragment.glsl:
/*
uniform lowp float u_device_pixel_ratio;
varying vec2 v_width2;
varying vec2 v_normal;
varying float v_gamma_scale;

#ifndef HAS_UNIFORM_u_color
varying highp vec4 color;
#else
uniform highp vec4 u_color;
#endif


#ifndef HAS_UNIFORM_u_blur
varying lowp float blur;
#else
uniform lowp float u_blur;
#endif


#ifndef HAS_UNIFORM_u_opacity
varying lowp float opacity;
#else
uniform lowp float u_opacity;
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

    float dist = length(v_normal) * v_width2.s;
    float blur2 = (blur + 1.0 / u_device_pixel_ratio) * v_gamma_scale;
    float alpha = clamp(min(dist - (v_width2.t - blur2), v_width2.s - dist) / blur2, 0.0, 1.0);
    gl_FragColor = color * (alpha * opacity);
#ifdef OVERDRAW_INSPECTOR
    gl_FragColor = vec4(1.0);
#endif
}
*/
// clang-format on
