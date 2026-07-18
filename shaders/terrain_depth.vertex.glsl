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

layout (location = 0) in vec4 a_pos; // xy = tile position, z = skirt flag (1 = skirt vertex)

uniform sampler2D u_dem;

void main() {
    // Same elevation displacement as the terrain shader (terrain.vertex.glsl),
    // including the skirt drop, rendering only depth for the symbol occlusion pass
    vec2 pos = a_pos.xy;

    float elevation = get_elevation(pos, u_dem, u_dem_tile_coords, u_unpack,
                                    u_dem_tile_coords.w, u_exaggeration, 1.0);

    float ele_delta = a_pos.z == 1.0 ? u_elevation_offset : 0.0;
    gl_Position = u_matrix * vec4(pos.x, pos.y, elevation - ele_delta, 1.0);
}
