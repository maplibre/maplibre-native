layout (std140) uniform BackgroundPropsUBO {
    highp vec4 u_color;
    highp float u_opacity;
    lowp float props_pad1;
    lowp float props_pad2;
    lowp float props_pad3;
};

void main() {
    fragColor = u_color * u_opacity;
#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
