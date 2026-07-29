#ifdef GL_ES
precision mediump float;
#else

#if !defined(lowp)
#define lowp
#endif

#if !defined(mediump)
#define mediump
#endif

#if !defined(highp)
#define highp
#endif

#endif

out highp vec4 fragColor;

// Terrain occlusion for 3D geometry, per fragment. Mirrors the vertex prelude's
// unpack_depth()/depth_opacity() (same convention: the terrain depth pass packs
// clip-space NDC z, so no window-depth remap), but usable from a fragment shader:
// a symbol fades as a whole label in the vertex stage, whereas an extruded building
// is real geometry that a ridge can cut through, so it must be tested per fragment.
float unpack_depth(vec4 rgba_depth) {
    const highp vec4 bit_shift = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
    return dot(rgba_depth, bit_shift);
}

// 1 = in front of the terrain (visible), 0 = behind it (hidden), with the same soft
// ramp and self-occlusion bias as the vertex-side depth_opacity().
highp float depth_opacity(vec3 frag, sampler2D depth_texture) {
    highp float d = unpack_depth(texture(depth_texture, frag.xy * 0.5 + 0.5)) + 0.0001 - frag.z;
    return 1.0 - max(0.0, min(1.0, -d * 5000.0));
}
