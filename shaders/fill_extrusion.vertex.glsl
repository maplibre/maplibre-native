layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_normal_ed;
layout (location = 2) in vec2 a_edge_distance;

out vec2 v_edge_dist;
out float v_z_height;
out vec3 v_normal;
out vec4 v_color;
out float v_t;

layout (std140) uniform FillExtrusionDrawableUBO {
    highp mat4 u_matrix;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp float u_height_factor;
    highp float u_tile_ratio;
    // Interpolations
    highp float u_base_t;
    highp float u_height_t;
    highp float u_color_t;
    highp float u_pattern_from_t;
    highp float u_pattern_to_t;
    lowp float drawable_pad1;
};

layout (std140) uniform FillExtrusionTilePropsUBO {
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
    highp vec2 u_texsize;
    lowp float tileprops_pad1;
    lowp float tileprops_pad2;
};

layout (std140) uniform FillExtrusionPropsUBO {
    highp vec4 u_color;
    highp vec3 u_lightcolor;
    lowp float props_pad1;
    highp vec3 u_lightpos;
    highp float u_base;
    highp float u_height;
    highp float u_lightintensity;
    highp float u_vertical_gradient;
    highp float u_opacity;
    highp float u_fade;
    highp float u_from_scale;
    highp float u_to_scale;
    highp float u_bevel_radius;
};

#pragma mapbox: define highp float base
#pragma mapbox: define highp float height
#pragma mapbox: define highp vec4 color

void main() {
    #pragma mapbox: initialize highp float base
    #pragma mapbox: initialize highp float height
    #pragma mapbox: initialize highp vec4 color

    vec3 normal = a_normal_ed.xyz;

    base = max(0.0, base);
    height = max(0.0, height);

    float t = mod(normal.x, 2.0);
    v_z_height = t > 0.0 ? height : base;
    v_edge_dist = a_edge_distance;
    v_normal = normal / 16384.0;
    v_t = t;

    gl_Position = u_matrix * vec4(a_pos, v_z_height, 1);

    // Add slight ambient lighting so no extrusions are totally black
    vec4 ambientlight = vec4(0.03, 0.03, 0.03, 1.0);
    v_color = color + ambientlight;
}
