layout (std140) uniform LineBasicUBO {
    highp mat4 u_matrix;
    highp vec2 u_units_to_pixels;
    mediump float u_ratio;
    lowp float u_device_pixel_ratio;
};

layout (std140) uniform LineBasicPropertiesUBO {
    highp vec4 u_color;
    lowp float u_opacity;
    mediump float u_width;

    highp vec2 pad1;
};

in float v_width;
in vec2 v_normal;
in float v_gamma_scale;

#pragma mapbox: define highp vec4 color
#pragma mapbox: define lowp float opacity

void main() {
    #pragma mapbox: initialize highp vec4 color
    #pragma mapbox: initialize lowp float opacity

    // Calculate the distance of the pixel from the line in pixels.
    float dist = length(v_normal) * v_width;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (v_width2.t) or when fading out
    // (v_width2.s)
    float blur2 = (1.0 / u_device_pixel_ratio) * v_gamma_scale;
    float alpha = clamp(min(dist + blur2, v_width - dist) / blur2, 0.0, 1.0);

    fragColor = color * (alpha * opacity);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
