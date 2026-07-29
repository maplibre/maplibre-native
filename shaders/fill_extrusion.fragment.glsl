in vec4 v_color;
in highp vec4 v_terrain_pos;
in mediump float v_depth_enabled;

uniform sampler2D u_terrain_depth;

void main() {
    fragColor = v_color;

    // Terrain occlusion: hide the parts of a building that fall behind the terrain.
    // Per fragment (not per drawable) so a building poking above a ridge keeps its
    // visible upper portion while the covered lower part is cut away. depth_opacity()
    // is the fragment-prelude twin of the symbol path's, so both use one convention.
    if (v_depth_enabled != 0.0) {
        highp vec3 frag = v_terrain_pos.xyz / v_terrain_pos.w;
        highp float visibility = depth_opacity(frag, u_terrain_depth);
        if (visibility <= 0.0) {
            discard;
        }
        fragColor.a *= visibility;
    }

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
