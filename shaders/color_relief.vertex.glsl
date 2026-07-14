layout (std140) uniform GlobalPaintParamsUBO {
    highp vec2 u_pattern_atlas_texsize;
    highp vec2 u_units_to_pixels;
    highp vec2 u_world_size;
    highp float u_camera_to_center_distance;
    highp float u_symbol_fade_change;
    highp float u_aspect_ratio;
    highp float u_pixel_ratio;
    highp float u_map_zoom;
    lowp float global_pad1;
    highp vec4 u_drape_tile;
};

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
    gl_Position = apply_drape_transform(gl_Position, u_matrix, u_drape_tile);

    highp vec2 epsilon = 1.0 / u_dimension;
    float scale = (u_dimension.x - 2.0) / u_dimension.x;
    v_pos = (a_texture_pos / 8192.0) * scale + epsilon;

    // Handle poles
    if (a_pos.y < -32767.5) v_pos.y = 0.0;
    if (a_pos.y > 32766.5) v_pos.y = 1.0;
}
