layout (std140) uniform BackgroundLayerUBO {
    highp vec4 u_color;
    highp float u_opacity;
    highp float pad1, pad2, pad3;
};

void main() {
    fragColor = u_color * u_opacity;
#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
