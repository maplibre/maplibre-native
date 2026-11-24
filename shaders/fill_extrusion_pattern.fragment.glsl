in vec2 v_pos_a;
in vec2 v_pos_b;
in vec4 v_lighting;
in highp float v_wall_z;
in vec3 v_lighting_without_ambient;

layout (std140) uniform FillExtrusionTilePropsUBO {
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
    highp vec2 u_texsize;
    lowp float tileprops_pad1;
    lowp float tileprops_pad2;
};

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

uniform sampler2D u_image;

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

#pragma mapbox: define lowp float base
#pragma mapbox: define lowp float height
#pragma mapbox: define mediump vec4 pattern_from
#pragma mapbox: define mediump vec4 pattern_to

void main() {
    #pragma mapbox: initialize lowp float base
    #pragma mapbox: initialize lowp float height
    #pragma mapbox: initialize mediump vec4 pattern_from
    #pragma mapbox: initialize mediump vec4 pattern_to

    vec2 pattern_tl_a = pattern_from.xy;
    vec2 pattern_br_a = pattern_from.zw;
    vec2 pattern_tl_b = pattern_to.xy;
    vec2 pattern_br_b = pattern_to.zw;

    vec2 imagecoord = mod(v_pos_a, 1.0);
    vec2 pos = mix(pattern_tl_a / u_texsize, pattern_br_a / u_texsize, imagecoord);
    vec4 color1 = texture(u_image, pos);

    vec2 imagecoord_b = mod(v_pos_b, 1.0);
    vec2 pos2 = mix(pattern_tl_b / u_texsize, pattern_br_b / u_texsize, imagecoord_b);
    vec4 color2 = texture(u_image, pos2);

    vec4 mixedColor = mix(color1, color2, u_fade);

    // Take ambient light away from the bottom of the walls. Both are premultiplied.
    vec4 lighting = vec4(mix(v_lighting.rgb, v_lighting_without_ambient, wallAmbientOcclusion(v_wall_z)), v_lighting.a);
    fragColor = mixedColor * lighting;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
