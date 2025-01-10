uniform sampler2D u_texture;

layout (std140) uniform SymbolTilePropsUBO {
    bool u_is_text;
    bool u_is_halo;
    highp float u_gamma_scale;
    lowp float tileprops_pad1;
};

layout (std140) uniform SymbolEvaluatedPropsUBO {
    highp vec4 u_text_fill_color;
    highp vec4 u_text_halo_color;
    highp float u_text_opacity;
    highp float u_text_halo_width;
    highp float u_text_halo_blur;
    lowp float props_pad1;
    highp vec4 u_icon_fill_color;
    highp vec4 u_icon_halo_color;
    highp float u_icon_opacity;
    highp float u_icon_halo_width;
    highp float u_icon_halo_blur;
    lowp float props_pad2;
};

in vec2 v_tex;
in float v_fade_opacity;

#pragma mapbox: define lowp float opacity

void main() {
    highp float u_opacity = u_is_text ? u_text_opacity : u_icon_opacity;

    #pragma mapbox: initialize lowp float opacity

    lowp float alpha = opacity * v_fade_opacity;
    fragColor = texture(u_texture, v_tex) * alpha;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
