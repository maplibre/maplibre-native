layout (std140) uniform FillPatternDrawableUBO {
    highp vec4 u_scale;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp vec2 u_texsize;
    highp float pad1;
    highp float pad2;
};
layout (std140) uniform FillPatternEvaluatedPropsUBO {
    highp float u_opacity;
    highp float u_fade;
    highp float padding_props1;
    highp float padding_props2;
};
layout (std140) uniform FillPatternInterpolateUBO {
    highp float u_pattern_from_t;
    highp float u_pattern_to_t;
    highp float u_opacity_t;
    highp float u_padding_interp1;
};
layout (std140) uniform FillPatternTilePropsUBO {
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
};

uniform sampler2D u_image;

in vec2 v_pos_a;
in vec2 v_pos_b;

#pragma mapbox: define lowp float opacity
#pragma mapbox: define lowp vec4 pattern_from
#pragma mapbox: define lowp vec4 pattern_to

void main() {
    #pragma mapbox: initialize lowp float opacity
    #pragma mapbox: initialize mediump vec4 pattern_from
    #pragma mapbox: initialize mediump vec4 pattern_to

    vec2 pattern_tl_a = pattern_from.xy;
    vec2 pattern_br_a = pattern_from.zw;
    vec2 pattern_tl_b = pattern_to.xy;
    vec2 pattern_br_b = pattern_to.zw;

    if (u_texsize.x < 1.0 || u_texsize.y < 1.0) {
        discard;
    }

    vec2 imagecoord = mod(v_pos_a, 1.0);
    vec2 pos = mix(pattern_tl_a / u_texsize, pattern_br_a / u_texsize, imagecoord);
    vec4 color1 = texture(u_image, pos);

    vec2 imagecoord_b = mod(v_pos_b, 1.0);
    vec2 pos2 = mix(pattern_tl_b / u_texsize, pattern_br_b / u_texsize, imagecoord_b);
    vec4 color2 = texture(u_image, pos2);

    fragColor = mix(color1, color2, u_fade) * opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
