layout (std140) uniform FillExtrusionShadowPropsUBO {
    highp vec4 u_color;
    highp float u_opacity;
    highp float u_base;
    highp float u_height;
};

void main() {
#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
    return;
#endif

    // u_color is already premultiplied, so scaling the whole vector keeps the alpha blend correct.
    fragColor = u_color * u_opacity;
}
