uniform sampler2D u_texture;

layout (std140) uniform SymbolDrawablePaintUBO {
    highp vec4 u_fill_color;
    highp vec4 u_halo_color;
    highp float u_opacity;
    highp float u_halo_width;
    highp float u_halo_blur;
    highp float u_padding;
};

layout (std140) uniform SymbolDrawableInterpolateUBO {
    highp float u_fill_color_t;
    highp float u_halo_color_t;
    highp float u_opacity_t;
    highp float u_halo_width_t;
    highp float u_halo_blur_t;
    highp float u_pad4,u_pad5,u_pad6;
};

in vec2 v_tex;
in float v_fade_opacity;

void main() {
    #pragma mapbox: initialize lowp float opacity

    lowp float alpha = opacity * v_fade_opacity;
    fragColor = texture(u_texture, v_tex) * alpha;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
