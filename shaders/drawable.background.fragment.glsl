layout (std140) uniform BackgroundLayerUBO {
    highp vec4 u_color;
    highp vec4 u_opacity_pad3;
};

void main() {
    fragColor = u_color * u_opacity_pad3.x;
#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
