uniform sampler2D u_texture;

layout (std140) uniform SymbolPaintUBO {
    highp vec4 u_fill_color;
    highp vec4 u_halo_color;
    highp float u_opacity;
    highp float u_halo_width;
    highp float u_halo_blur;
    highp float paint_pad1;
};

layout (std140) uniform SymbolInterpolateUBO {
    highp float u_fill_color_t;
    highp float u_halo_color_t;
    highp float u_opacity_t;
    highp float u_halo_width_t;
    highp float u_halo_blur_t;
    highp float interp_pad1, interp_pad2,interp_pad3;
};

in vec2 v_tex;
in float v_fade_opacity;

#pragma mapbox: define lowp float opacity

void main() {
    #pragma mapbox: initialize lowp float opacity

    lowp float alpha = opacity * v_fade_opacity;
    fragColor = texture(u_texture, v_tex) * alpha;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
