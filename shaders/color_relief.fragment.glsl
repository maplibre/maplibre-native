layout(std140) uniform ColorReliefDrawableUBO {
    highp mat4 u_matrix;
};

layout(std140) uniform ColorReliefTilePropsUBO {
    highp vec4 u_unpack;
    highp vec2 u_dimension;
    int u_color_ramp_size;
    float pad_tile0;
};

layout(std140) uniform ColorReliefEvaluatedPropsUBO {
    float u_opacity;
    float pad_eval0;
    float pad_eval1;
    float pad_eval2;
};

uniform sampler2D u_image;
uniform sampler2D u_elevation_stops;
uniform sampler2D u_color_stops;

in vec2 v_pos;

float getElevation(vec2 coord) {
    // Convert encoded elevation value to meters
    vec4 data = texture(u_image, coord) * 255.0;
    data.a = -1.0;
    return dot(data, u_unpack);
}

float getElevationStop(int stop) {
    // Elevation stops are plain float values, not terrain-RGB encoded
    float x = (float(stop) + 0.5) / float(u_color_ramp_size);
    return texture(u_elevation_stops, vec2(x, 0.0)).g;
}

vec4 getColorStop(int stop) {
    float x = (float(stop) + 0.5) / float(u_color_ramp_size);
    return texture(u_color_stops, vec2(x, 0.0));
}

void main() {
    // --- TEMPORARY DEBUG CODE START ---

    // Sample the middle of the u_elevation_stops texture.
    vec4 raw_data = texture(u_elevation_stops, vec2(0.5, 0.0));
    
    // Normalize the values to the display range [0.0, 1.0].
    // Using 3000.0 (the max elevation from your logs) as a normalization factor.
    float normalize_factor = 1.0 / 3000.0;
    
    // Output the raw R, G, B, A components of the sampled vec4, scaled down.
    // The screen's color will reveal which channel contains the data.
    fragColor = raw_data * normalize_factor; // <--- Correct assignment for GLES 3.0

    // --- TEMPORARY DEBUG CODE END ---
}