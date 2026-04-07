out vec2 v_uv;
out float v_elevation;

layout (std140) uniform TerrainDrawableUBO {
    highp mat4 u_matrix;
};

layout (std140) uniform TerrainEvaluatedPropsUBO {
    highp float u_exaggeration;
};

layout (location = 0) in vec2 a_pos;

uniform sampler2D u_dem;

void main() {
    // Convert vertex position to normalized texture coordinates [0, 1]
    // The mesh was generated with coordinates from 0 to EXTENT (8192)
    vec2 pos = vec2(a_pos);
    v_uv = pos / 8192.0;

    // Sample DEM texture to get raw RGBA values
    vec4 demSample = texture(u_dem, v_uv);

    // Decode Mapbox Terrain RGB format to get elevation in meters
    // Format: height = -10000 + ((R*256*256 + G*256 + B) * 0.1)
    // DEM values are in range [0, 1] so convert back to [0, 255]
    float r = demSample.r * 255.0;
    float g = demSample.g * 255.0;
    float b = demSample.b * 255.0;

    // Calculate elevation in meters
    float elevationMeters = -10000.0 + ((r * 256.0 * 256.0 + g * 256.0 + b) * 0.1);

    // Apply exaggeration for visible relief (default: 1.0, can be set higher for dramatic effect)
    v_elevation = elevationMeters * u_exaggeration;

    // Create 3D position with elevation as Z coordinate
    gl_Position = u_matrix * vec4(pos.x, pos.y, v_elevation, 1.0);
}
