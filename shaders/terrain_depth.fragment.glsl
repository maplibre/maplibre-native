in highp float v_depth;
void main() {
    // v_depth is clip-space NDC z (gl_Position.z / w). Pack the window depth [0, 1] into RGBA8
    // as four base-256 digits, each written as k / 255 so the unorm8 target stores it exactly
    // (the gl-js fract/bit_mask scheme stores k / 256 and loses up to 0.002 to unorm rounding).
    // Decoded by unpack_depth() in the vertex prelude for calculate_visibility().
    highp float r = clamp(v_depth * 0.5 + 0.5, 0.0, 0.99999994) * 256.0;
    highp float d0 = floor(r);
    r = (r - d0) * 256.0;
    highp float d1 = floor(r);
    r = (r - d1) * 256.0;
    highp float d2 = floor(r);
    r = (r - d2) * 256.0;
    highp float d3 = floor(r);
    fragColor = vec4(d0, d1, d2, d3) / 255.0;
}
