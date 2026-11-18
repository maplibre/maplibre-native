in vec2 v_uv;
in float v_elevation;
in highp float v_depth;

layout (std140) uniform TerrainEvaluatedPropsUBO {
    highp vec4 u_unpack;
    highp float u_exaggeration;
    highp float u_elevation_offset;
    lowp float props_pad1;
    lowp float props_pad2;
    highp vec4 u_haze_color; // premultiplied; clear when there is no haze
    highp vec2 u_haze_ramp; // haze = clamp(x - y / view depth, 0, 1)^2
    lowp float props_pad3;
    lowp float props_pad4;
};

uniform sampler2D u_map;

void main() {
    // Sample the map texture (render-to-texture output) for the surface color
    // Note: Y-coordinate is flipped (1.0 - y) to match OpenGL convention
    fragColor = texture(u_map, vec2(v_uv.x, 1.0 - v_uv.y));

    // Distance haze: past where it begins the ground fades toward the haze color, as gl-js's terrain fog
    // does, to the full color (its alpha) where it ends (the ramp is TerrainLayerTweaker's). A zero
    // ramp leaves the drape as it is.
    highp float haze = clamp(u_haze_ramp.x - u_haze_ramp.y / max(v_depth, 1e-6), 0.0, 1.0);
    haze *= haze;
    fragColor.rgb = fragColor.rgb * (1.0 - u_haze_color.a * haze) + u_haze_color.rgb * (fragColor.a * haze);

#if defined(OVERDRAW_INSPECTOR)
    fragColor = vec4(1.0);
#endif
}
