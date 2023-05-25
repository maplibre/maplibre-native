layout (std140) uniform FillDrawableUBO {
    highp mat4 u_matrix;
    highp vec4 u_scale;
    highp vec2 u_world;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp vec2 u_texsize;
    highp float u_fade;

    highp float u_color_t;
    highp float u_opacity_t;
    highp float u_outline_color_t;
    highp float u_pattern_from_t;
    highp float u_pattern_to_t;

    highp vec2 u_color;
    highp vec2 u_opacity;
    highp vec2 u_outline_color_pad;
    highp vec4 u_outline_color;
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
};

#pragma mapbox: define highp vec4 color
#pragma mapbox: define lowp float opacity

void main() {
    #pragma mapbox: initialize highp vec4 color
    #pragma mapbox: initialize lowp float opacity

    fragColor = color * opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
