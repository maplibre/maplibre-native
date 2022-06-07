// NOTE: DO NOT CHANGE THIS FILE. IT IS AUTOMATICALLY GENERATED.
// clang-format off
#include <mbgl/programs/line_sdf_program.hpp>
#include <mbgl/programs/gl/preludes.hpp>
#include <mbgl/programs/gl/shader_source.hpp>
#include <mbgl/gl/program.hpp>

namespace mbgl {
namespace programs {
namespace gl {

template <typename>
struct ShaderSource;

template <>
struct ShaderSource<LineSDFProgram> {
    static constexpr const char* name = "line_sdf";
    static constexpr const uint8_t hash[8] = {0xe2, 0x5e, 0x17, 0x38, 0x2f, 0xe1, 0xbf, 0x79};
    static constexpr const auto vertexOffset = 50110;
    static constexpr const auto fragmentOffset = 53847;
};

constexpr const char* ShaderSource<LineSDFProgram>::name;
constexpr const uint8_t ShaderSource<LineSDFProgram>::hash[8];

} // namespace gl
} // namespace programs

namespace gfx {

template <>
std::unique_ptr<gfx::Program<LineSDFProgram>>
Backend::Create<gfx::Backend::Type::OpenGL>(const ProgramParameters& programParameters) {
    return std::make_unique<gl::Program<LineSDFProgram>>(programParameters);
}

} // namespace gfx
} // namespace mbgl

// Uncompressed source of line_sdf.vertex.glsl:
/*

#define scale 0.015873016
#define LINE_DISTANCE_SCALE 2.0
attribute vec2 a_pos_normal;
attribute vec4 a_data;
uniform mat4 u_matrix;
uniform mediump float u_ratio;
uniform lowp float u_device_pixel_ratio;
uniform vec2 u_patternscale_a;
uniform float u_tex_y_a;
uniform vec2 u_patternscale_b;
uniform float u_tex_y_b;
uniform vec2 u_units_to_pixels;
varying vec2 v_normal;
varying vec2 v_width2;
varying vec2 v_tex_a;
varying vec2 v_tex_b;
varying float v_gamma_scale;

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
varying mediump float width;
#else
uniform mediump float u_width;
#endif


#ifndef HAS_UNIFORM_u_floorwidth
uniform lowp float u_floorwidth_t;
attribute lowp vec2 a_floorwidth;
varying lowp float floorwidth;
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

    float ANTIALIASING = 1.0 / u_device_pixel_ratio / 2.0;
    vec2 a_extrude = a_data.xy - 128.0;
    float a_direction = mod(a_data.z, 4.0) - 1.0;
    float a_linesofar = (floor(a_data.z / 4.0) + a_data.w * 64.0) * LINE_DISTANCE_SCALE;
    vec2 pos = floor(a_pos_normal * 0.5);
    mediump vec2 normal = a_pos_normal - 2.0 * pos;
    normal.y = normal.y * 2.0 - 1.0;
    v_normal = normal;
    gapwidth = gapwidth / 2.0;
    float halfwidth = width / 2.0;
    offset = -1.0 * offset;
    float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);
    mediump vec2 dist =outset * a_extrude * scale;
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
    v_tex_a = vec2(a_linesofar * u_patternscale_a.x / floorwidth, normal.y * u_patternscale_a.y + u_tex_y_a);
    v_tex_b = vec2(a_linesofar * u_patternscale_b.x / floorwidth, normal.y * u_patternscale_b.y + u_tex_y_b);
    v_width2 = vec2(outset, inset);
}
*/

// Uncompressed source of line_sdf.fragment.glsl:
/*
uniform lowp float u_device_pixel_ratio;
uniform sampler2D u_image;
uniform float u_sdfgamma;
uniform float u_mix;
varying vec2 v_normal;
varying vec2 v_width2;
varying vec2 v_tex_a;
varying vec2 v_tex_b;
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


#ifndef HAS_UNIFORM_u_width
varying mediump float width;
#else
uniform mediump float u_width;
#endif


#ifndef HAS_UNIFORM_u_floorwidth
varying lowp float floorwidth;
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

    float dist = length(v_normal) * v_width2.s;
    float blur2 = (blur + 1.0 / u_device_pixel_ratio) * v_gamma_scale;
    float alpha = clamp(min(dist - (v_width2.t - blur2), v_width2.s - dist) / blur2, 0.0, 1.0);
    float sdfdist_a = texture2D(u_image, v_tex_a).a;
    float sdfdist_b = texture2D(u_image, v_tex_b).a;
    float sdfdist = mix(sdfdist_a, sdfdist_b, u_mix);
    alpha *= smoothstep(0.5 - u_sdfgamma / floorwidth, 0.5 + u_sdfgamma / floorwidth, sdfdist);
    gl_FragColor = color * (alpha * opacity);
#ifdef OVERDRAW_INSPECTOR
    gl_FragColor = vec4(1.0);
#endif
}
*/
// clang-format on
