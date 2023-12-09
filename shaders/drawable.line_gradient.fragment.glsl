layout (std140) uniform LineGradientUBO {
    highp mat4 u_matrix;
    highp vec2 u_units_to_pixels;
    mediump float u_ratio;
    lowp float pad0;
};

layout (std140) uniform LineGradientPropertiesUBO {
    lowp float u_blur;
    lowp float u_opacity;
    mediump float u_gapwidth;
    lowp float u_offset;
    mediump float u_width;

    highp float pad1;
    highp vec2 pad2;
};

layout (std140) uniform LineGradientInterpolationUBO {
    lowp float u_blur_t;
    lowp float u_opacity_t;
    lowp float u_gapwidth_t;
    lowp float u_offset_t;
    lowp float u_width_t;

    highp float pad3;
    highp vec2 pad4;
};

uniform sampler2D u_image;

in vec2 v_width2;
in vec2 v_normal;
in float v_gamma_scale;
in highp float v_lineprogress;

#pragma mapbox: define lowp float blur
#pragma mapbox: define lowp float opacity

void main() {
    #pragma mapbox: initialize lowp float blur
    #pragma mapbox: initialize lowp float opacity

    // Calculate the distance of the pixel from the line in pixels.
    float dist = length(v_normal) * v_width2.s;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (v_width2.t) or when fading out
    // (v_width2.s)
    float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * v_gamma_scale;
    float alpha = clamp(min(dist - (v_width2.t - blur2), v_width2.s - dist) / blur2, 0.0, 1.0);

    // For gradient lines, v_lineprogress is the ratio along the entire line,
    // scaled to [0, 2^15), and the gradient ramp is stored in a texture.
    vec4 color = texture(u_image, vec2(v_lineprogress, 0.5));

    fragColor = color * (alpha * opacity);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
