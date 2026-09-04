layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_anchor_pos;
layout (location = 2) in vec2 a_extrude;
layout (location = 3) in vec2 a_placed;
layout (location = 4) in vec2 a_shift;

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

layout (std140) uniform CollisionDrawableUBO {
    highp mat4 u_matrix;
};

layout (std140) uniform CollisionTilePropsUBO {
    highp vec2 u_extrude_scale;
    highp float u_overscale_factor;
    lowp float drawable_pad1;
};

out float v_placed;
out float v_notUsed;

void main() {
#ifdef PROJECTION_GLOBE
    vec4 projectedPoint = projectTile(a_anchor_pos);
#else
    vec4 projectedPoint = u_matrix * vec4(a_anchor_pos, 0, 1);
#endif
    highp float camera_to_anchor_distance = projectedPoint.w;
    highp float collision_perspective_ratio = clamp(
        0.5 + 0.5 * (u_camera_to_center_distance / camera_to_anchor_distance),
        0.0, // Prevents oversized near-field boxes in pitched/overzoomed tiles
        4.0);

#ifdef PROJECTION_GLOBE
    gl_Position = projectTile(a_pos);
    // Boxes just past the horizon stay visible, as in GL JS; ones far behind it are clipped.
    if (gl_Position.z / gl_Position.w < 1.1) {
        gl_Position.z = 0.0;
    }
#else
    gl_Position = u_matrix * vec4(a_pos, 0.0, 1.0);
#endif
    gl_Position.xy += (a_extrude + a_shift) * u_extrude_scale * gl_Position.w * collision_perspective_ratio;

    v_placed = a_placed.x;
    v_notUsed = a_placed.y;
}
