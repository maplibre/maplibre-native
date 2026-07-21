in vec2 v_uv;
in float v_elevation;

uniform sampler2D u_map;

void main() {
    // Sample the map texture (render-to-texture output) for the surface color
    // Note: Y-coordinate is flipped (1.0 - y) to match OpenGL convention
    fragColor = texture(u_map, vec2(v_uv.x, 1.0 - v_uv.y));

#if defined(OVERDRAW_INSPECTOR)
    fragColor = vec4(1.0);
#endif
}
