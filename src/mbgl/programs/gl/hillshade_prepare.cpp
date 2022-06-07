// NOTE: DO NOT CHANGE THIS FILE. IT IS AUTOMATICALLY GENERATED.
// clang-format off
#include <mbgl/programs/hillshade_prepare_program.hpp>
#include <mbgl/programs/gl/preludes.hpp>
#include <mbgl/programs/gl/shader_source.hpp>
#include <mbgl/gl/program.hpp>

namespace mbgl {
namespace programs {
namespace gl {

template <typename>
struct ShaderSource;

template <>
struct ShaderSource<HillshadePrepareProgram> {
    static constexpr const char* name = "hillshade_prepare";
    static constexpr const uint8_t hash[8] = {0xd9, 0xb5, 0x98, 0xed, 0x2b, 0x45, 0x55, 0xef};
    static constexpr const auto vertexOffset = 32461;
    static constexpr const auto fragmentOffset = 32754;
};

constexpr const char* ShaderSource<HillshadePrepareProgram>::name;
constexpr const uint8_t ShaderSource<HillshadePrepareProgram>::hash[8];

} // namespace gl
} // namespace programs

namespace gfx {

template <>
std::unique_ptr<gfx::Program<HillshadePrepareProgram>>
Backend::Create<gfx::Backend::Type::OpenGL>(const ProgramParameters& programParameters) {
    return std::make_unique<gl::Program<HillshadePrepareProgram>>(programParameters);
}

} // namespace gfx
} // namespace mbgl

// Uncompressed source of hillshade_prepare.vertex.glsl:
/*
uniform mat4 u_matrix;
uniform vec2 u_dimension;
attribute vec2 a_pos;
attribute vec2 a_texture_pos;
varying vec2 v_pos;
void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    highp vec2 epsilon = 1.0 / u_dimension;
    float scale = (u_dimension.x - 2.0) / u_dimension.x;
    v_pos = (a_texture_pos / 8192.0) * scale + epsilon;
}
*/

// Uncompressed source of hillshade_prepare.fragment.glsl:
/*
#ifdef GL_ES
precision highp float;
#endif
uniform sampler2D u_image;
varying vec2 v_pos;
uniform vec2 u_dimension;
uniform float u_zoom;
uniform vec4 u_unpack;
float getElevation(vec2 coord, float bias) {
    vec4 data = texture2D(u_image, coord) * 255.0;
    data.a = -1.0;
    return dot(data, u_unpack) / 4.0;
}
void main() {
    vec2 epsilon = 1.0 / u_dimension;
    float a = getElevation(v_pos + vec2(-epsilon.x, -epsilon.y), 0.0);
    float b = getElevation(v_pos + vec2(0, -epsilon.y), 0.0);
    float c = getElevation(v_pos + vec2(epsilon.x, -epsilon.y), 0.0);
    float d = getElevation(v_pos + vec2(-epsilon.x, 0), 0.0);
    float e = getElevation(v_pos, 0.0);
    float f = getElevation(v_pos + vec2(epsilon.x, 0), 0.0);
    float g = getElevation(v_pos + vec2(-epsilon.x, epsilon.y), 0.0);
    float h = getElevation(v_pos + vec2(0, epsilon.y), 0.0);
    float i = getElevation(v_pos + vec2(epsilon.x, epsilon.y), 0.0);
    float exaggerationFactor = u_zoom < 2.0 ? 0.4 : u_zoom < 4.5 ? 0.35 : 0.3;
    float exaggeration = u_zoom < 15.0 ? (u_zoom - 15.0) * exaggerationFactor : 0.0;
    vec2 deriv = vec2(
        (c + f + f + i) - (a + d + d + g),
        (g + h + h + i) - (a + b + b + c)
    ) / pow(2.0, exaggeration + (19.2562 - u_zoom));
    gl_FragColor = clamp(vec4(
        deriv.x / 2.0 + 0.5,
        deriv.y / 2.0 + 0.5,
        1.0,
        1.0), 0.0, 1.0);
#ifdef OVERDRAW_INSPECTOR
    gl_FragColor = vec4(1.0);
#endif
}
*/
// clang-format on
