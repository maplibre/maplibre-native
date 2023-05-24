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

in vec2 v_pos;

#pragma mapbox: define highp vec4 outline_color
#pragma mapbox: define lowp float opacity

void main() {
    #pragma mapbox: initialize highp vec4 outline_color
    #pragma mapbox: initialize lowp float opacity

    float dist = length(v_pos - gl_FragCoord.xy);
    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    fragColor = outline_color * (alpha * opacity);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
