void main() {
    // Pack the fragment depth into RGBA8, as in the maplibre-gl-js
    // terrain_depth shader; unpacked by unpack_depth() in the vertex prelude
    // for calculate_visibility()
    highp float depth = gl_FragCoord.z;
    const highp vec4 bit_shift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
    const highp vec4 bit_mask = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    highp vec4 res = fract(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    fragColor = res;
}
