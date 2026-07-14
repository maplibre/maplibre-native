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
    // Convert vertex position to normalized texture coordinates [0, 1]
    // The mesh was generated with coordinates from 0 to EXTENT (8192)
    vec2 pos = vec2(a_pos);
    v_uv = pos / 8192.0;

    // Sample the DEM texture and decode elevation in meters using the source's
    // unpack vector, matching hillshade/color-relief (supports Mapbox Terrain-RGB
    // and Terrarium encodings)
    // Map into the bound DEM tile; an ancestor tile is bound as a fallback
    // while this tile's own DEM is still loading
    vec2 dem_uv = v_uv * u_dem_tile_coords.x + u_dem_tile_coords.yz;
    vec4 demSample = texture(u_dem, dem_uv) * 255.0;
    demSample.a = -1.0;
    float elevationMeters = dot(demSample, u_unpack);

    // Apply exaggeration for visible relief (default: 1.0, can be set higher for dramatic effect)
    v_elevation = elevationMeters * u_exaggeration;

    // Create 3D position with elevation as Z coordinate
    gl_Position = u_matrix * vec4(pos.x, pos.y, v_elevation, 1.0);
}
