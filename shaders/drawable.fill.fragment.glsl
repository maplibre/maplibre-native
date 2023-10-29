layout (std140) uniform FillEvaluatedPropsUBO {
    highp vec4 u_color;
    highp float u_opacity;
    highp float padding_props1;
    highp float padding_props2;
    highp float padding_props3;
};
layout (std140) uniform FillInterpolateUBO {
    highp float u_color_t;
    highp float u_opacity_t;
    highp float pad_interp_1, pad_interp_2;
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
