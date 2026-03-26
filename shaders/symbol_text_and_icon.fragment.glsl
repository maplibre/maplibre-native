#define SDF_PX 8.0

#define SDF 1.0
#define ICON 0.0

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

uniform sampler2D u_texture;
uniform sampler2D u_texture_icon;

in vec4 v_data0;
in vec4 v_data1;

#pragma mapbox: define highp vec4 fill_color
#pragma mapbox: define highp vec4 halo_color
#pragma mapbox: define lowp float opacity
#pragma mapbox: define lowp float halo_width
#pragma mapbox: define lowp float halo_blur

void main() {
    highp vec4 u_fill_color = u_is_text ? u_text_fill_color : u_icon_fill_color;
    highp vec4 u_halo_color = u_is_text ? u_text_halo_color : u_icon_halo_color;
    highp float u_opacity = u_is_text ? u_text_opacity : u_icon_opacity;
    highp float u_halo_width = u_is_text ? u_text_halo_width : u_icon_halo_width;
    highp float u_halo_blur = u_is_text ? u_text_halo_blur : u_icon_halo_blur;

    #pragma mapbox: initialize highp vec4 fill_color
    #pragma mapbox: initialize highp vec4 halo_color
    #pragma mapbox: initialize lowp float opacity
    #pragma mapbox: initialize lowp float halo_width
    #pragma mapbox: initialize lowp float halo_blur

    float fade_opacity = v_data1[2];

    if (v_data1.w == ICON) {
        vec2 tex_icon = v_data0.zw;
        lowp float alpha = opacity * fade_opacity;
        fragColor = texture(u_texture_icon, tex_icon) * alpha;

#ifdef OVERDRAW_INSPECTOR
        fragColor = vec4(1.0);
#endif
        return;
    }

    vec2 tex = v_data0.xy;

    float EDGE_GAMMA = 0.105 / DEVICE_PIXEL_RATIO;

    float gamma_scale = v_data1.x;
    float size = v_data1.y;

    float fontScale = size / 24.0;

    lowp vec4 color = fill_color;
    highp float gamma = EDGE_GAMMA / (fontScale * u_gamma_scale);
    lowp float inner_edge = (256.0 - 64.0) / 256.0;
    if (u_is_halo) {
        color = halo_color;
        gamma = (halo_blur * 1.19 / SDF_PX + EDGE_GAMMA) / (fontScale * u_gamma_scale);
        inner_edge = inner_edge + gamma * gamma_scale;
    }

    lowp float dist = texture(u_texture, tex).a;
    highp float gamma_scaled = gamma * gamma_scale;
    highp float alpha = smoothstep(inner_edge - gamma_scaled, inner_edge + gamma_scaled, dist);
    if (u_is_halo) {
        // When drawing halos, we want the inside of the halo to be transparent as well
        // in case the text fill is transparent.
        lowp float halo_edge = (6.0 - halo_width / fontScale) / SDF_PX;
        alpha = min(smoothstep(halo_edge - gamma_scaled, halo_edge + gamma_scaled, dist), 1.0 - alpha);
    }

    fragColor = color * (alpha * opacity * fade_opacity);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
