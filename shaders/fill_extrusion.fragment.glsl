in vec4 v_color;
in highp float v_wall_z;

layout (std140) uniform FillExtrusionPropsUBO {
    highp vec4 u_color;
    highp vec3 u_lightcolor;
    highp float u_ambient_occlusion_wall_intensity;
    highp vec3 u_lightpos;
    highp float u_base;
    highp float u_height;
    highp float u_lightintensity;
    highp float u_vertical_gradient;
    highp float u_opacity;
    highp float u_fade;
    highp float u_from_scale;
    highp float u_to_scale;
    highp float u_ambient_occlusion_wall_radius;
};

// Ambient occlusion of a wall at `z` meters above the ground: 1 leaves the color as is, lower values darken it.
// Smooth, and fully faded out at the radius. Roofs are left alone.
highp float wallAmbientOcclusion(highp float z) {
    if (z < 0.0 || u_ambient_occlusion_wall_intensity <= 0.0 || u_ambient_occlusion_wall_radius <= 0.0) {
        return 1.0;
    }
    return 1.0 - u_ambient_occlusion_wall_intensity * (1.0 - smoothstep(0.0, u_ambient_occlusion_wall_radius, z));
}

void main() {
    // Darken the bottom of the walls. The color is premultiplied, so only scale its rgb.
    fragColor = vec4(v_color.rgb * wallAmbientOcclusion(v_wall_z), v_color.a);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
