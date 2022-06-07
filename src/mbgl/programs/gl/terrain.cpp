// NOTE: DO NOT CHANGE THIS FILE. IT IS AUTOMATICALLY GENERATED.
// clang-format off
#include <mbgl/programs/terrain_program.hpp>
#include <mbgl/programs/gl/preludes.hpp>
#include <mbgl/programs/gl/shader_source.hpp>
#include <mbgl/gl/program.hpp>

namespace mbgl {
namespace programs {
namespace gl {

template <typename>
struct ShaderSource;

template <>
struct ShaderSource<TerrainProgram> {
    static constexpr const char* name = "terrain";
    static constexpr const uint8_t hash[8] = {0x95, 0x13, 0x54, 0xaa, 0x1c, 0xd4, 0x9a, 0x84};
    static constexpr const auto vertexOffset = 72425;
    static constexpr const auto fragmentOffset = 72654;
};

constexpr const char* ShaderSource<TerrainProgram>::name;
constexpr const uint8_t ShaderSource<TerrainProgram>::hash[8];

} // namespace gl
} // namespace programs

namespace gfx {

template <>
std::unique_ptr<gfx::Program<TerrainProgram>>
Backend::Create<gfx::Backend::Type::OpenGL>(const ProgramParameters& programParameters) {
    return std::make_unique<gl::Program<TerrainProgram>>(programParameters);
}

} // namespace gfx
} // namespace mbgl

// Uncompressed source of terrain.vertex.glsl:
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

// Uncompressed source of terrain.fragment.glsl:
/*
uniform sampler2D u_texture;
varying vec2 v_texture_pos;
void main() {
    gl_FragColor = texture2D(u_texture, v_texture_pos);
}
*/
// clang-format on
