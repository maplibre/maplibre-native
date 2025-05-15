layout (std140) uniform BackgroundPatternDrawableUBO {
    highp mat4 u_matrix;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp float u_tile_units_to_pixels;
    lowp float drawable_pad1;
    lowp float drawable_pad2;
    lowp float drawable_pad3;
};

layout (std140) uniform BackgroundPatternPropsUBO {
    highp vec2 u_pattern_tl_a;
    highp vec2 u_pattern_br_a;
    highp vec2 u_pattern_tl_b;
    highp vec2 u_pattern_br_b;
    highp vec2 u_pattern_size_a;
    highp vec2 u_pattern_size_b;
    highp float u_scale_a;
    highp float u_scale_b;
    highp float u_mix;
    highp float u_opacity;
};

layout (location = 0) in vec2 a_pos;
out mediump vec2 v_pos_a;
out mediump vec2 v_pos_b;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);

    v_pos_a = get_pattern_pos(u_pixel_coord_upper, u_pixel_coord_lower, u_scale_a * u_pattern_size_a, u_tile_units_to_pixels, a_pos);
    v_pos_b = get_pattern_pos(u_pixel_coord_upper, u_pixel_coord_lower, u_scale_b * u_pattern_size_b, u_tile_units_to_pixels, a_pos);
}
