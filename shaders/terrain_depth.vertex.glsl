#define TERRAIN_MAX_INSTANCES 64

// One per instance: the tile's clip matrix, its DEM sub-tile coords ({scale, xoff, yoff,
// dem_dim}) and the sampler2DArray layer holding that tile's (or an ancestor's) DEM. The
// whole array is bound as the TerrainDrawableUBO block and indexed per instance; matches
// TerrainDepthInstanceUBO on the C++ side (std140, 6*16 bytes each).
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
    // Instanced depth pass: one draw covers every terrain mesh tile; gl_InstanceID selects this
    // instance's tile data. Same elevation displacement (incl. skirt drop) as terrain.vertex,
    // rendering only packed depth for symbol occlusion (calculate_visibility).
    //
    // We use the built-in gl_InstanceID rather than a per-instance vertex attribute: the GL
    // backend keeps all program attributes in vertexAttributes and always returns an empty
    // getInstanceAttributes(), so an instance attribute (divisor 1) is never actually bound -
    // it reads the default 0 for every instance. The C++ instance-attribute array is still set
    // on the drawable, but only as the instance-count source (DrawableGL::draw reads its
    // getMinCount()); the index itself comes from gl_InstanceID.
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
