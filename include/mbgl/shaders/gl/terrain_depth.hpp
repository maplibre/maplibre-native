// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::TerrainDepthShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "TerrainDepthShader";
    static constexpr const char* vertex = R"(layout (std140) uniform TerrainDrawableUBO {
    highp mat4 u_matrix;
    highp vec4 u_dem_tile_coords;
};

layout (std140) uniform TerrainEvaluatedPropsUBO {
    highp vec4 u_unpack;
    highp float u_exaggeration;
    highp float u_elevation_offset;
    lowp float props_pad1;
    lowp float props_pad2;
};

layout (location = 0) in vec2 a_pos;

uniform sampler2D u_dem;

void main() {
    // Same elevation displacement as the terrain shader (terrain.vertex.glsl),
    // rendering only depth for the symbol occlusion pass
    vec2 pos = vec2(a_pos);
    vec2 uv = pos / 8192.0;

    vec2 dem_uv = uv * u_dem_tile_coords.x + u_dem_tile_coords.yz;
    vec4 demSample = texture(u_dem, dem_uv) * 255.0;
    demSample.a = -1.0;
    float elevation = dot(demSample, u_unpack) * u_exaggeration;

    gl_Position = u_matrix * vec4(pos.x, pos.y, elevation, 1.0);
}
)";
    static constexpr const char* fragment = R"(void main() {
    // Pack the fragment depth into RGBA8, as in the maplibre-gl-js
    // terrain_depth shader; unpacked by unpack_depth() in the vertex prelude
    // for calculate_visibility()
    highp float depth = gl_FragCoord.z;
    const highp vec4 bit_shift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
    const highp vec4 bit_mask = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    highp vec4 res = fract(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    fragColor = res;
}
)";
};

} // namespace shaders
} // namespace mbgl
