// Generated code, do not modify this file!
#pragma once
#include <mln/shaders/shader_source.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::TerrainShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "TerrainShader";
    static constexpr const char* vertex = R"(out vec2 v_uv;
out float v_elevation;

layout (std140) uniform TerrainDrawableUBO {
    highp mat4 u_matrix;
    highp vec4 u_dem_tile_coords;
    highp float u_skirt_edges; // edges with a neighbor tile: north 1, east 2, south 4, west 8
    lowp float drawable_pad1;
    lowp float drawable_pad2;
    lowp float drawable_pad3;
};

layout (std140) uniform TerrainEvaluatedPropsUBO {
    highp vec4 u_unpack;
    highp float u_exaggeration;
    highp float u_elevation_offset;
    lowp float props_pad1;
    lowp float props_pad2;
};

layout (location = 0) in vec4 a_pos; // xy = tile position, z = skirt flag (1 = skirt vertex)

uniform sampler2D u_dem;

void main() {
    // The mesh was generated with coordinates from 0 to EXTENT (8192)
    vec2 pos = a_pos.xy;
    v_uv = pos / 8192.0;

    // Decode the DEM and interpolate in meters via the shared prelude helper
    // (the packed Terrain-RGB/Terrarium DEM cannot be hardware-filtered, so it is
    // sampled NEAREST and interpolated after decoding, matching maplibre-gl-js and
    // the elevated layers). Map into the bound DEM tile; an ancestor tile is bound
    // as a fallback while this tile's own DEM loads. dem_coords.w = DEM dimension.
    v_elevation = get_elevation(pos, u_dem, u_dem_tile_coords, u_unpack,
                                u_dem_tile_coords.w, u_exaggeration, 1.0);

    // Skirt vertices hang below the surface by u_elevation_offset, forming a
    // curtain that hides the cracks between neighbouring tiles at different zoom
    // levels (maplibre-gl-js u_ele_delta).
    // A skirt vertex on an edge with no neighbor tile stays on the surface, so a corner hangs only
    // where both of its edges have one.
    int edges = int(u_skirt_edges);
    bool bare = (pos.y == 0.0 && (edges & 1) == 0) || (pos.x == 8192.0 && (edges & 2) == 0) ||
                (pos.y == 8192.0 && (edges & 4) == 0) || (pos.x == 0.0 && (edges & 8) == 0);
    float ele_delta = a_pos.z == 1.0 && !bare ? u_elevation_offset : 0.0;
    gl_Position = u_matrix * vec4(pos.x, pos.y, v_elevation - ele_delta, 1.0);
}
)";
    static constexpr const char* fragment = R"(in vec2 v_uv;
in float v_elevation;

uniform sampler2D u_map;

void main() {
    // Sample the map texture (render-to-texture output) for the surface color
    // Note: Y-coordinate is flipped (1.0 - y) to match OpenGL convention
    fragColor = texture(u_map, vec2(v_uv.x, 1.0 - v_uv.y));

#if defined(OVERDRAW_INSPECTOR)
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mln
