// NOTE: DO NOT CHANGE THIS FILE. IT IS AUTOMATICALLY GENERATED.
// clang-format off
#include <mbgl/programs/fill_extrusion_program.hpp>
#include <mbgl/programs/gl/preludes.hpp>
#include <mbgl/programs/gl/shader_source.hpp>
#include <mbgl/gl/program.hpp>

namespace mbgl {
namespace programs {
namespace gl {

template <typename>
struct ShaderSource;

template <>
struct ShaderSource<FillExtrusionProgram> {
    static constexpr const char* name = "fill_extrusion";
    static constexpr const uint8_t hash[8] = {0x6a, 0x6a, 0xe2, 0x3e, 0xe7, 0x1d, 0x71, 0x1f};
    static constexpr const auto vertexOffset = 24609;
    static constexpr const auto fragmentOffset = 26734;
};

constexpr const char* ShaderSource<FillExtrusionProgram>::name;
constexpr const uint8_t ShaderSource<FillExtrusionProgram>::hash[8];

} // namespace gl
} // namespace programs

namespace gfx {

template <>
std::unique_ptr<gfx::Program<FillExtrusionProgram>>
Backend::Create<gfx::Backend::Type::OpenGL>(const ProgramParameters& programParameters) {
    return std::make_unique<gl::Program<FillExtrusionProgram>>(programParameters);
}

} // namespace gfx
} // namespace mbgl

// Uncompressed source of fill_extrusion.vertex.glsl:
/*
uniform mat4 u_matrix;
uniform vec3 u_lightcolor;
uniform lowp vec3 u_lightpos;
uniform lowp float u_lightintensity;
uniform float u_vertical_gradient;
uniform lowp float u_opacity;
attribute vec2 a_pos;
attribute vec4 a_normal_ed;
#ifdef TERRAIN3D
    attribute vec2 a_centroid;
#endif
varying vec4 v_color;

#ifndef HAS_UNIFORM_u_base
uniform lowp float u_base_t;
attribute highp vec2 a_base;
#else
uniform highp float u_base;
#endif


#ifndef HAS_UNIFORM_u_height
uniform lowp float u_height_t;
attribute highp vec2 a_height;
#else
uniform highp float u_height;
#endif


#ifndef HAS_UNIFORM_u_color
uniform lowp float u_color_t;
attribute highp vec4 a_color;
#else
uniform highp vec4 u_color;
#endif

void main() {
    
#ifndef HAS_UNIFORM_u_base
    highp float base = unpack_mix_vec2(a_base, u_base_t);
#else
    highp float base = u_base;
#endif

    
#ifndef HAS_UNIFORM_u_height
    highp float height = unpack_mix_vec2(a_height, u_height_t);
#else
    highp float height = u_height;
#endif

    
#ifndef HAS_UNIFORM_u_color
    highp vec4 color = unpack_mix_color(a_color, u_color_t);
#else
    highp vec4 color = u_color;
#endif

    vec3 normal = a_normal_ed.xyz;
    #ifdef TERRAIN3D
        float baseDelta = 10.0;
        float ele = get_elevation(a_centroid);
    #else
        float baseDelta = 0.0;
        float ele = 0.0;
    #endif
    base = max(0.0, ele + base - baseDelta);
    height = max(0.0, ele + height);
    float t = mod(normal.x, 2.0);
    gl_Position = u_matrix * vec4(a_pos, t > 0.0 ? height : base, 1);
    float colorvalue = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
    v_color = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 ambientlight = vec4(0.03, 0.03, 0.03, 1.0);
    color += ambientlight;
    float directional = clamp(dot(normal / 16384.0, u_lightpos), 0.0, 1.0);
    directional = mix((1.0 - u_lightintensity), max((1.0 - colorvalue + u_lightintensity), 1.0), directional);
    if (normal.y != 0.0) {
        directional *= (
            (1.0 - u_vertical_gradient) +
            (u_vertical_gradient * clamp((t + base) * pow(height / 150.0, 0.5), mix(0.7, 0.98, 1.0 - u_lightintensity), 1.0)));
    }
    v_color.r += clamp(color.r * directional * u_lightcolor.r, mix(0.0, 0.3, 1.0 - u_lightcolor.r), 1.0);
    v_color.g += clamp(color.g * directional * u_lightcolor.g, mix(0.0, 0.3, 1.0 - u_lightcolor.g), 1.0);
    v_color.b += clamp(color.b * directional * u_lightcolor.b, mix(0.0, 0.3, 1.0 - u_lightcolor.b), 1.0);
    v_color *= u_opacity;
}
*/

// Uncompressed source of fill_extrusion.fragment.glsl:
/*
varying vec4 v_color;
void main() {
    gl_FragColor = v_color;
#ifdef OVERDRAW_INSPECTOR
    gl_FragColor = vec4(1.0);
#endif
}
*/
// clang-format on
