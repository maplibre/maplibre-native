// NOTE: DO NOT CHANGE THIS FILE. IT IS AUTOMATICALLY GENERATED.
// clang-format off
#include <mbgl/programs/heatmap_program.hpp>
#include <mbgl/programs/gl/preludes.hpp>
#include <mbgl/programs/gl/shader_source.hpp>
#include <mbgl/gl/program.hpp>

namespace mbgl {
namespace programs {
namespace gl {

template <typename>
struct ShaderSource;

template <>
struct ShaderSource<HeatmapProgram> {
    static constexpr const char* name = "heatmap";
    static constexpr const uint8_t hash[8] = {0xe5, 0xa4, 0x9c, 0x31, 0x01, 0xe5, 0x4a, 0xe0};
    static constexpr const auto vertexOffset = 9654;
    static constexpr const auto fragmentOffset = 10702;
};

constexpr const char* ShaderSource<HeatmapProgram>::name;
constexpr const uint8_t ShaderSource<HeatmapProgram>::hash[8];

} // namespace gl
} // namespace programs

namespace gfx {

template <>
std::unique_ptr<gfx::Program<HeatmapProgram>>
Backend::Create<gfx::Backend::Type::OpenGL>(const ProgramParameters& programParameters) {
    return std::make_unique<gl::Program<HeatmapProgram>>(programParameters);
}

} // namespace gfx
} // namespace mbgl

// Uncompressed source of heatmap.vertex.glsl:
/*
uniform mat4 u_matrix;
uniform float u_extrude_scale;
uniform float u_opacity;
uniform float u_intensity;
attribute vec2 a_pos;
varying vec2 v_extrude;

#ifndef HAS_UNIFORM_u_weight
uniform lowp float u_weight_t;
attribute highp vec2 a_weight;
varying highp float weight;
#else
uniform highp float u_weight;
#endif


#ifndef HAS_UNIFORM_u_radius
uniform lowp float u_radius_t;
attribute mediump vec2 a_radius;
#else
uniform mediump float u_radius;
#endif

const highp float ZERO = 1.0 / 255.0 / 16.0;
#define GAUSS_COEF 0.3989422804014327
void main(void) {
    
#ifndef HAS_UNIFORM_u_weight
    weight = unpack_mix_vec2(a_weight, u_weight_t);
#else
    highp float weight = u_weight;
#endif

    
#ifndef HAS_UNIFORM_u_radius
    mediump float radius = unpack_mix_vec2(a_radius, u_radius_t);
#else
    mediump float radius = u_radius;
#endif

    vec2 unscaled_extrude = vec2(mod(a_pos, 2.0) * 2.0 - 1.0);
    float S = sqrt(-2.0 * log(ZERO / weight / u_intensity / GAUSS_COEF)) / 3.0;
    v_extrude = S * unscaled_extrude;
    vec2 extrude = v_extrude * radius * u_extrude_scale;
    vec4 pos = vec4(floor(a_pos * 0.5) + extrude, 0, 1);
    gl_Position = u_matrix * pos;
}
*/

// Uncompressed source of heatmap.fragment.glsl:
/*
uniform highp float u_intensity;
varying vec2 v_extrude;

#ifndef HAS_UNIFORM_u_weight
varying highp float weight;
#else
uniform highp float u_weight;
#endif

#define GAUSS_COEF 0.3989422804014327
void main() {
    
#ifdef HAS_UNIFORM_u_weight
    highp float weight = u_weight;
#endif

    float d = -0.5 * 3.0 * 3.0 * dot(v_extrude, v_extrude);
    float val = weight * u_intensity * GAUSS_COEF * exp(d);
    gl_FragColor = vec4(val, 1.0, 1.0, 1.0);
#ifdef OVERDRAW_INSPECTOR
    gl_FragColor = vec4(1.0);
#endif
}
*/
// clang-format on
