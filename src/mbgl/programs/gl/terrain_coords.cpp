// NOTE: DO NOT CHANGE THIS FILE. IT IS AUTOMATICALLY GENERATED.
// clang-format off
#include <mbgl/programs/terrain_coords_program.hpp>
#include <mbgl/programs/gl/preludes.hpp>
#include <mbgl/programs/gl/shader_source.hpp>
#include <mbgl/gl/program.hpp>

namespace mbgl {
namespace programs {
namespace gl {

template <typename>
struct ShaderSource;

template <>
struct ShaderSource<TerrainCoordsProgram> {
    static constexpr const char* name = "terrain_coords";
    static constexpr const uint8_t hash[8] = {0xaf, 0x20, 0xb5, 0xcc, 0xac, 0x0c, 0x0a, 0x81};
    static constexpr const auto vertexOffset = 73284;
    static constexpr const auto fragmentOffset = 73513;
};

constexpr const char* ShaderSource<TerrainCoordsProgram>::name;
constexpr const uint8_t ShaderSource<TerrainCoordsProgram>::hash[8];

} // namespace gl
} // namespace programs

namespace gfx {

template <>
std::unique_ptr<gfx::Program<TerrainCoordsProgram>>
Backend::Create<gfx::Backend::Type::OpenGL>(const ProgramParameters& programParameters) {
    return std::make_unique<gl::Program<TerrainCoordsProgram>>(programParameters);
}

} // namespace gfx
} // namespace mbgl

// Uncompressed source of terrain_coords.vertex.glsl:
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

// Uncompressed source of terrain_coords.fragment.glsl:
/*
precision mediump float;
uniform sampler2D u_texture;
uniform float u_terrain_coords_id;
varying vec2 v_texture_pos;
void main() {
   vec4 rgba = texture2D(u_texture, v_texture_pos);
   gl_FragColor = vec4(rgba.r, rgba.g, rgba.b, u_terrain_coords_id);
}
*/
// clang-format on
