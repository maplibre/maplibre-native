in vec4 v_color;
in highp float v_wall_z;
in vec3 v_color_without_ambient;

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

// Ambient occlusion of a wall at `z` meters above the ground: the share of the ambient light that is taken away,
// 0 leaves the color as is. (1 - s)^1.5, with s the height over the radius, follows the share of the rays that hit
// the ground within the radius. Roofs are left alone.
highp float wallAmbientOcclusion(highp float z) {
    if (z < 0.0 || u_ambient_occlusion_wall_intensity <= 0.0 || u_ambient_occlusion_wall_radius <= 0.0) {
        return 0.0;
    }
    highp float s = clamp(z / u_ambient_occlusion_wall_radius, 0.0, 1.0);
    return u_ambient_occlusion_wall_intensity * (1.0 - s) * sqrt(1.0 - s);
}

void main() {
    // Take ambient light away from the bottom of the walls. Both colors are premultiplied.
    fragColor = vec4(mix(v_color.rgb, v_color_without_ambient, wallAmbientOcclusion(v_wall_z)), v_color.a);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
