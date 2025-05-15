layout (std140) uniform RasterEvaluatedPropsUBO {
    highp vec3 u_spin_weights;
    highp vec2 u_tl_parent;
    highp float u_scale_parent;
    highp float u_buffer_scale;
    highp float u_fade_t;
    highp float u_opacity;
    highp float u_brightness_low;
    highp float u_brightness_high;
    highp float u_saturation_factor;
    highp float u_contrast_factor;
    lowp float props_pad1;
    lowp float props_pad2;
};
uniform sampler2D u_image0;
uniform sampler2D u_image1;

in vec2 v_pos0;
in vec2 v_pos1;

void main() {

    // read and cross-fade colors from the main and parent tiles
    vec4 color0 = texture(u_image0, v_pos0);
    vec4 color1 = texture(u_image1, v_pos1);
    if (color0.a > 0.0) {
        color0.rgb = color0.rgb / color0.a;
    }
    if (color1.a > 0.0) {
        color1.rgb = color1.rgb / color1.a;
    }
    vec4 color = mix(color0, color1, u_fade_t);
    color.a *= u_opacity;
    vec3 rgb = color.rgb;

    // spin
    rgb = vec3(
        dot(rgb, u_spin_weights.xyz),
        dot(rgb, u_spin_weights.zxy),
        dot(rgb, u_spin_weights.yzx));

    // saturation
    float average = (color.r + color.g + color.b) / 3.0;
    rgb += (average - rgb) * u_saturation_factor;

    // contrast
    rgb = (rgb - 0.5) * u_contrast_factor + 0.5;

    // brightness
    vec3 u_high_vec = vec3(u_brightness_low, u_brightness_low, u_brightness_low);
    vec3 u_low_vec = vec3(u_brightness_high, u_brightness_high, u_brightness_high);

    fragColor = vec4(mix(u_high_vec, u_low_vec, rgb) * color.a, color.a);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
