layout (std140) uniform DrawableUBO {
    mat4 u_matrix;
    vec2 u_world;
    vec2 u_padding_drawable;
};
layout (std140) uniform FillLayerUBO {
    vec4 u_scale;
    vec2 u_pixel_coord_upper;
    vec2 u_pixel_coord_lower;
    vec2 u_texsize;
    float u_fade;
    float u_color_t;
    float u_opacity_t;
    float u_outline_color_t;
    float u_pattern_from_t;
    float u_pattern_to_t;
};

layout (location = 0) in vec2 a_pos;

#pragma mapbox: define highp vec4 color
#pragma mapbox: define lowp float opacity

void main() {
    #pragma mapbox: initialize highp vec4 color
    #pragma mapbox: initialize lowp float opacity

    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
