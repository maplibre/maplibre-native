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
};

layout (std140) uniform FillOutlineDrawableUBO {
    highp mat4 u_matrix;
    // Interpolations
    highp float u_outline_color_t;
    highp float u_opacity_t;
    lowp float u_fade_zoom_t;
    lowp float drawable_pad2;
};

layout (std140) uniform FillEvaluatedPropsUBO {
    highp vec4 u_color;
    highp vec4 u_outline_color;
    highp float u_opacity;
    highp float u_rounded_corner_distance;
    highp float u_fade;
    highp float u_from_scale;
    highp float u_to_scale;
    highp float u_fade_zoom;
    lowp float props_pad2;
    lowp float props_pad3;
};

layout (std140) uniform FillFadeZoomUBO {
    highp vec4 u_fade_zoom_table[104];
};

out lowp float v_fade_zoom_opacity;

layout (location = 0) in vec2 a_pos;

out vec2 v_pos;

#pragma mapbox: define highp vec4 outline_color
#pragma mapbox: define lowp float opacity
#pragma mapbox: define highp float fade_zoom

void main() {
    #pragma mapbox: initialize highp vec4 outline_color
    #pragma mapbox: initialize lowp float opacity
    #pragma mapbox: initialize highp float fade_zoom

    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    v_pos = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * u_world_size;

    // Fade opacity for this fade zoom: one table entry per 1/16 of a zoom level, packed four to a vec4.
    int fade_zoom_index = clamp(int(floor(fade_zoom * 16.0 + 0.5)), 0, 415);
    v_fade_zoom_opacity = u_fade_zoom_table[fade_zoom_index / 4][fade_zoom_index % 4];
}
