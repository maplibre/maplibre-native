// NOTE: DO NOT CHANGE THIS FILE. IT IS AUTOMATICALLY GENERATED.
// clang-format off
#include <mbgl/programs/terrain_depth_program.hpp>
#include <mbgl/programs/gl/preludes.hpp>
#include <mbgl/programs/gl/shader_source.hpp>
#include <mbgl/gl/program.hpp>

namespace mbgl {
namespace programs {
namespace gl {

template <typename>
struct ShaderSource;

template <>
struct ShaderSource<TerrainDepthProgram> {
    static constexpr const char* name = "terrain_depth";
    static constexpr const uint8_t hash[8] = {0xfd, 0xdf, 0xc7, 0xdf, 0xd9, 0x5d, 0x90, 0xb5};
    static constexpr const auto vertexOffset = 72773;
    static constexpr const auto fragmentOffset = 73002;
};

constexpr const char* ShaderSource<TerrainDepthProgram>::name;
constexpr const uint8_t ShaderSource<TerrainDepthProgram>::hash[8];

} // namespace gl
} // namespace programs

namespace gfx {

template <>
std::unique_ptr<gfx::Program<TerrainDepthProgram>>
Backend::Create<gfx::Backend::Type::OpenGL>(const ProgramParameters& programParameters) {
    return std::make_unique<gl::Program<TerrainDepthProgram>>(programParameters);
}

} // namespace gfx
} // namespace mbgl

// Uncompressed source of terrain_depth.vertex.glsl:
/*
attribute vec2 a_pos;
uniform mat4 u_matrix;
varying vec2 v_texture_pos;
varying float v_depth;
void main() {
    v_texture_pos = a_pos / 8192.0;
    gl_Position = u_matrix * vec4(a_pos, get_elevation(a_pos), 1.0);
    v_depth = gl_Position.z / gl_Position.w;
}
*/

// Uncompressed source of terrain_depth.fragment.glsl:
/*
varying float v_depth;
const highp vec4 bitSh = vec4(256. * 256. * 256., 256. * 256., 256., 1.);
const highp vec4 bitMsk = vec4(0.,vec3(1./256.0));
highp vec4 pack(highp float value) {
    highp vec4 comp = fract(value * bitSh);
    comp -= comp.xxyz * bitMsk;
    return comp;
}
void main() {
    gl_FragColor = pack(v_depth);
}
*/
// clang-format on
