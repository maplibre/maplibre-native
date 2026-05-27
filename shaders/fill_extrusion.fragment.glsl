in vec2 v_edge_dist;
in float v_z_height;
in vec3 v_normal;
in vec4 v_color;
in float v_t;

layout (std140) uniform FillExtrusionPropsUBO {
    highp vec4 u_color;
    highp vec3 u_lightcolor;
    lowp float props_pad1;
    highp vec3 u_lightpos;
    highp float u_base;
    highp float u_height;
    highp float u_lightintensity;
    highp float u_vertical_gradient;
    highp float u_opacity;
    highp float u_fade;
    highp float u_from_scale;
    highp float u_to_scale;
    highp float u_bevel_radius;
};

void main() {
    vec3 flat_normal = v_normal;

    // Fake bevel: bend normals near roof edges toward the up-vector
    vec3 roof_up = vec3(0.0, 0.0, 1.0);
    float roof_curve = smoothstep(u_bevel_radius, 0.0, v_edge_dist.y);
    vec3 beveled_normal = mix(flat_normal, roof_up, roof_curve);

    // Keep ground-level footprint edges sharp
    float bottom_sharpness = smoothstep(0.0, u_bevel_radius, v_z_height);
    vec3 final_normal = normalize(mix(flat_normal, beveled_normal, bottom_sharpness));

    // Relative luminance (how dark/bright is the surface color?)
    float colorvalue = v_color.r * 0.2126 + v_color.g * 0.7152 + v_color.b * 0.0722;

    vec4 frag_color = vec4(0.0, 0.0, 0.0, 1.0);

    // Calculate cos(theta), where theta is the angle between surface normal and diffuse light ray
    float directional = clamp(dot(final_normal, u_lightpos), 0.0, 1.0);

    // Adjust directional so that
    // the range of values for highlight/shading is narrower
    // with lower light intensity
    // and with lighter/brighter surface colors
    directional = mix((1.0 - u_lightintensity), max((1.0 - colorvalue + u_lightintensity), 1.0), directional);

    // Add gradient along z axis of side surfaces
    if (v_normal.y != 0.0) {
        directional *= (
            (1.0 - u_vertical_gradient) +
            (u_vertical_gradient * clamp((v_t + u_base) * pow(u_height / 150.0, 0.5), mix(0.7, 0.98, 1.0 - u_lightintensity), 1.0)));
    }

    // Assign final color based on surface + ambient light color, diffuse light directional, and light color
    // with lower bounds adjusted to hue of light
    // so that shading is tinted with the complementary (opposite) color to the light color
    frag_color.r += clamp(v_color.r * directional * u_lightcolor.r, mix(0.0, 0.3, 1.0 - u_lightcolor.r), 1.0);
    frag_color.g += clamp(v_color.g * directional * u_lightcolor.g, mix(0.0, 0.3, 1.0 - u_lightcolor.g), 1.0);
    frag_color.b += clamp(v_color.b * directional * u_lightcolor.b, mix(0.0, 0.3, 1.0 - u_lightcolor.b), 1.0);
    fragColor = frag_color * u_opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
