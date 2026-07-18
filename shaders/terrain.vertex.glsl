out vec2 v_uv;
out float v_elevation;

layout (std140) uniform TerrainDrawableUBO {
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
    // The mesh was generated with coordinates from 0 to EXTENT (8192)
    vec2 pos = vec2(a_pos);
    v_uv = pos / 8192.0;

    // Decode the DEM and interpolate in meters via the shared prelude helper
    // (the packed Terrain-RGB/Terrarium DEM cannot be hardware-filtered, so it is
    // sampled NEAREST and interpolated after decoding, matching maplibre-gl-js and
    // the elevated layers). Map into the bound DEM tile; an ancestor tile is bound
    // as a fallback while this tile's own DEM loads. dem_coords.w = DEM dimension.
    v_elevation = get_elevation(pos, u_dem, u_dem_tile_coords, u_unpack,
                                u_dem_tile_coords.w, u_exaggeration, 1.0);

    // Create 3D position with elevation as Z coordinate
    gl_Position = u_matrix * vec4(pos.x, pos.y, v_elevation, 1.0);
}
