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

layout (std140) uniform CollisionTilePropsUBO {
    highp vec2 u_extrude_scale;
    highp float u_overscale_factor;
    lowp float drawable_pad1;
};

in float v_placed;
in float v_notUsed;
in float v_radius;
in highp vec2 v_extrude;
in vec2 v_extrude_scale;

void main() {
    float alpha = 0.5;

    // Red = collision, hide label
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0) * alpha;

    // Blue = no collision, label is showing
    if (v_placed > 0.5) {
        color = vec4(0.0, 0.0, 1.0, 0.5) * alpha;
    }

    if (v_notUsed > 0.5) {
        // This box not used, fade it out
        color *= .2;
    }

    float extrude_scale_length = length(v_extrude_scale);
    float extrude_length = length(v_extrude) * extrude_scale_length;
    float stroke_width = 15.0 * extrude_scale_length / u_overscale_factor;
    float radius = v_radius * extrude_scale_length;

    float distance_to_edge = abs(extrude_length - radius);
    float opacity_t = smoothstep(-stroke_width, 0.0, -distance_to_edge);

    fragColor = opacity_t * color;
}
