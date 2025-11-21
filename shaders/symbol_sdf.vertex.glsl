layout (location = 0) in vec4 a_pos_offset;
layout (location = 1) in vec4 a_data;
layout (location = 2) in vec4 a_pixeloffset;
layout (location = 3) in vec3 a_projected_pos;
layout (location = 4) in float a_fade_opacity;

// contents of a_size vary based on the type of property value
// used for {text,icon}-size.
// For constants, a_size is disabled.
// For source functions, we bind only one value per vertex: the value of {text,icon}-size evaluated for the current feature.
// For composite functions:
// [ text-size(lowerZoomStop, feature),
//   text-size(upperZoomStop, feature) ]

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

layout (std140) uniform SymbolDrawableUBO {
    highp mat4 u_matrix;
    highp mat4 u_label_plane_matrix;
    highp mat4 u_coord_matrix;

    highp vec2 u_texsize;
    highp vec2 u_texsize_icon;

    bool u_is_text_prop;
    bool u_rotate_symbol;
    bool u_pitch_with_map;
    bool u_is_size_zoom_constant;
    bool u_is_size_feature_constant;
    bool u_is_offset;

    highp float u_size_t; // used to interpolate between zoom stops when size is a composite function
    highp float u_size; // used when size is both zoom and feature constant

    // Interpolations
    highp float u_fill_color_t;
    highp float u_halo_color_t;
    highp float u_opacity_t;
    highp float u_halo_width_t;
    highp float u_halo_blur_t;
};

layout (std140) uniform SymbolEvaluatedPropsUBO {
    highp vec4 u_text_fill_color;
    highp vec4 u_text_halo_color;
    highp float u_text_opacity;
    highp float u_text_halo_width;
    highp float u_text_halo_blur;
    lowp float props_pad1;
    highp vec4 u_icon_fill_color;
    highp vec4 u_icon_halo_color;
    highp float u_icon_opacity;
    highp float u_icon_halo_width;
    highp float u_icon_halo_blur;
    lowp float props_pad2;
};

out vec2 v_data0;
out vec3 v_data1;

#pragma mapbox: define highp vec4 fill_color
#pragma mapbox: define highp vec4 halo_color
#pragma mapbox: define lowp float opacity
#pragma mapbox: define lowp float halo_width
#pragma mapbox: define lowp float halo_blur

void main() {
    highp vec4 u_fill_color = u_is_text_prop ? u_text_fill_color : u_icon_fill_color;
    highp vec4 u_halo_color = u_is_text_prop ? u_text_halo_color : u_icon_halo_color;
    highp float u_opacity = u_is_text_prop ? u_text_opacity : u_icon_opacity;
    highp float u_halo_width = u_is_text_prop ? u_text_halo_width : u_icon_halo_width;
    highp float u_halo_blur = u_is_text_prop ? u_text_halo_blur : u_icon_halo_blur;

    #pragma mapbox: initialize highp vec4 fill_color
    #pragma mapbox: initialize highp vec4 halo_color
    #pragma mapbox: initialize lowp float opacity
    #pragma mapbox: initialize lowp float halo_width
    #pragma mapbox: initialize lowp float halo_blur

    vec2 a_pos = a_pos_offset.xy;
    vec2 a_offset = a_pos_offset.zw;

    vec2 a_tex = a_data.xy;
    vec2 a_size = a_data.zw;

    float a_size_min = floor(a_size[0] * 0.5);
    vec2 a_pxoffset = a_pixeloffset.xy;

    highp float segment_angle = -a_projected_pos[2];
    float size;

    if (!u_is_size_zoom_constant && !u_is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], u_size_t) / 128.0;
    } else if (u_is_size_zoom_constant && !u_is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = u_size;
    }

    vec4 projectedPoint = u_matrix * vec4(a_pos, 0, 1);
    highp float camera_to_anchor_distance = projectedPoint.w;
    // If the label is pitched with the map, layout is done in pitched space,
    // which makes labels in the distance smaller relative to viewport space.
    // We counteract part of that effect by multiplying by the perspective ratio.
    // If the label isn't pitched with the map, we do layout in viewport space,
    // which makes labels in the distance larger relative to the features around
    // them. We counteract part of that effect by dividing by the perspective ratio.
    highp float distance_ratio = u_pitch_with_map ?
        camera_to_anchor_distance / u_camera_to_center_distance :
        u_camera_to_center_distance / camera_to_anchor_distance;
    highp float perspective_ratio = clamp(
        0.5 + 0.5 * distance_ratio,
        0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
        4.0);

    if (!u_is_offset) {
        size *= perspective_ratio;
    }

    float fontScale = u_is_text_prop ? size / 24.0 : size;

    highp float symbol_rotation = 0.0;
    if (u_rotate_symbol) {
        // Point labels with 'rotation-alignment: map' are horizontal with respect to tile units
        // To figure out that angle in projected space, we draw a short horizontal line in tile
        // space, project it, and measure its angle in projected space.
        vec4 offsetProjectedPoint = u_matrix * vec4(a_pos + vec2(1, 0), 0, 1);

        vec2 a = projectedPoint.xy / projectedPoint.w;
        vec2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;

        symbol_rotation = atan((b.y - a.y) / u_aspect_ratio, b.x - a.x);
    }

    highp float angle_sin = sin(segment_angle + symbol_rotation);
    highp float angle_cos = cos(segment_angle + symbol_rotation);
    mat2 rotation_matrix = mat2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    vec4 projected_pos = u_label_plane_matrix * vec4(a_projected_pos.xy, 0.0, 1.0);
    gl_Position = u_coord_matrix * vec4(projected_pos.xy / projected_pos.w + rotation_matrix * (a_offset / 32.0 * fontScale + a_pxoffset), 0.0, 1.0);
    float gamma_scale = gl_Position.w;

    vec2 fade_opacity = unpack_opacity(a_fade_opacity);
    float fade_change = fade_opacity[1] > 0.5 ? u_symbol_fade_change : -u_symbol_fade_change;
    float interpolated_fade_opacity = max(0.0, min(1.0, fade_opacity[0] + fade_change));

    v_data0 = a_tex / u_texsize;
    v_data1 = vec3(gamma_scale, size, interpolated_fade_opacity);
}
