// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::TerrainDepthShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "TerrainDepthShader";
    static constexpr const char* vertex = R"(#define TERRAIN_MAX_INSTANCES 64

struct TerrainInstance {
    highp mat4 matrix;
    highp vec4 dem_coords;
    highp float dem_layer;
    highp float pad1;
    highp float pad2;
    highp float pad3;
};

layout (std140) uniform TerrainDrawableUBO {
    TerrainInstance u_inst[TERRAIN_MAX_INSTANCES];
};

layout (std140) uniform TerrainEvaluatedPropsUBO {
    highp vec4 u_unpack;
    highp float u_exaggeration;
    highp float u_elevation_offset;
    lowp float props_pad1;
    lowp float props_pad2;
};

layout (location = 0) in vec4 a_pos; // xy = tile position, z = skirt flag (1 = skirt vertex)

uniform highp sampler2DArray u_dem_array;

out highp float v_depth;

void main() {
    // gl_InstanceID selects this instance's tile slot. A per-instance vertex attribute is not
    // usable on the GL backend (getInstanceAttributes() is always empty, so divisor 1 is never
    // bound and the attribute reads 0 for every instance); the built-in index is authoritative.
    int idx = gl_InstanceID;
    highp mat4 matrix = u_inst[idx].matrix;
    highp vec4 dem_coords = u_inst[idx].dem_coords;
    highp float dem_layer = u_inst[idx].dem_layer;

    vec2 pos = a_pos.xy;

    float elevation = get_elevation_array(pos, u_dem_array, dem_layer, dem_coords,
                                          u_unpack, dem_coords.w, u_exaggeration);

    float ele_delta = a_pos.z == 1.0 ? u_elevation_offset : 0.0;
    gl_Position = matrix * vec4(pos.x, pos.y, elevation - ele_delta, 1.0);
    // Carry clip-space NDC z (not gl_FragCoord.z window depth) so the packed value is
    // independent of glDepthRange and matches the symbol's frag.z = pos.z/pos.w, exactly
    // as maplibre-gl-js terrain_depth does (v_depth = gl_Position.z / gl_Position.w).
    v_depth = gl_Position.z / gl_Position.w;
}
)";
    static constexpr const char* fragment = R"(in highp float v_depth;
void main() {
    // Pack the clip-space NDC depth into RGBA8, as in the maplibre-gl-js
    // terrain_depth shader; unpacked by unpack_depth() in the vertex prelude
    // for calculate_visibility()
    highp float depth = v_depth;
    const highp vec4 bit_shift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
    const highp vec4 bit_mask = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    highp vec4 res = fract(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    fragColor = res;
}
)";
};

} // namespace shaders
} // namespace mln
