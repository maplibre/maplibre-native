layout(std140) uniform ColorReliefDrawableUBO {
    highp mat4 u_matrix;
};

layout(std140) uniform ColorReliefTilePropsUBO {
    highp vec4 u_unpack;
    highp vec2 u_dimension;
    highp int u_color_ramp_size;
    highp float pad_tile0;
};

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;
out vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);

    // With a 3-pixel DEM border (stride = dim+6), the displayed tile area
    // maps to interior cells at buffer indices 3..dim+2. Sample at the
    // border/interior boundary on each side so the bilinear blend pulls
    // in shared neighbour data at the edge — matches the seam-aware
    // sampling used by the hillshade-color shader.
    float scale = (u_dimension.x - 6.0) / u_dimension.x;
    v_pos = (a_texture_pos / 8192.0) * scale + 3.0 / u_dimension.x;

    // Handle poles
    if (a_pos.y < -32767.5) v_pos.y = 0.0;
    if (a_pos.y > 32766.5) v_pos.y = 1.0;
}
