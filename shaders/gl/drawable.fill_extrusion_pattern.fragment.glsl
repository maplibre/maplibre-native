in vec2 v_pos_a;
in vec2 v_pos_b;
in vec4 v_lighting;

layout (std140) uniform FillExtrusionDrawableTilePropsUBO {
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
};
layout (std140) uniform FillExtrusionInterpolateUBO {
    highp float u_base_t;
    highp float u_height_t;
    highp float u_color_t;
    highp float u_pattern_from_t;
    highp float u_pattern_to_t;
    highp float u_pad_interp1, u_pad_interp2, u_pad_interp3;
};
layout (std140) uniform FillExtrusionDrawableUBO {
    highp mat4 u_matrix;
    highp vec4 u_scale;
    highp vec2 u_texsize;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp float u_height_factor;
    highp float u_pad_drawable;
};
layout (std140) uniform FillExtrusionDrawablePropsUBO {
    highp vec4 u_color;
    highp vec3 u_lightcolor;
    highp float u_pad1;
    highp vec3 u_lightpos;
    highp float u_base;
    highp float u_height;
    highp float u_lightintensity;
    highp float u_vertical_gradient;
    highp float u_opacity;
    highp float u_fade;
    highp float u_pad_props2, u_pad_props3, u_pad_props4;
};

uniform sampler2D u_image;

#pragma mapbox: define lowp float base
#pragma mapbox: define lowp float height
#pragma mapbox: define mediump vec4 pattern_from
#pragma mapbox: define mediump vec4 pattern_to

void main() {
    #pragma mapbox: initialize lowp float base
    #pragma mapbox: initialize lowp float height
    #pragma mapbox: initialize mediump vec4 pattern_from
    #pragma mapbox: initialize mediump vec4 pattern_to

    vec2 pattern_tl_a = pattern_from.xy;
    vec2 pattern_br_a = pattern_from.zw;
    vec2 pattern_tl_b = pattern_to.xy;
    vec2 pattern_br_b = pattern_to.zw;

    vec2 imagecoord = mod(v_pos_a, 1.0);
    vec2 pos = mix(pattern_tl_a / u_texsize, pattern_br_a / u_texsize, imagecoord);
    vec4 color1 = texture(u_image, pos);

    vec2 imagecoord_b = mod(v_pos_b, 1.0);
    vec2 pos2 = mix(pattern_tl_b / u_texsize, pattern_br_b / u_texsize, imagecoord_b);
    vec4 color2 = texture(u_image, pos2);

    vec4 mixedColor = mix(color1, color2, u_fade);

    fragColor = mixedColor * v_lighting;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
