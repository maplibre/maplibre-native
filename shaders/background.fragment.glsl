layout (std140) uniform BackgroundPropsUBO {
    highp vec4 u_color;
    highp float u_opacity;
    highp float u_pitch;
    highp float u_horizon_clip_y;
    highp float u_viewport_height;
};

void main() {
    highp vec4 ground = u_color * u_opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
    return;
#endif

    // Screen-space NDC Y (bottom = -1, top = +1). Tile drawables use projected
    // vertex positions; comparing clip-space varyings misplaces the sky band when pitched.
    highp float screenNdcY = 2.0 * gl_FragCoord.y / max(u_viewport_height, 1.0) - 1.0;

    if (u_pitch < 0.01 || screenNdcY <= u_horizon_clip_y) {
        fragColor = ground;
        return;
    }

    // Solid light blue sky above the horizon.
    fragColor = vec4(0.5294117647058824, 0.807843137254902, 0.9215686274509803, 1.0) * u_opacity;
}
