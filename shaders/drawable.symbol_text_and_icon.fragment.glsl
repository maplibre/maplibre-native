#define SDF_PX 8.0

#define SDF 1.0
#define ICON 0.0

layout (std140) uniform SymbolDrawableUBO {
    highp mat4 u_matrix;
    highp mat4 u_label_plane_matrix;
    highp mat4 u_coord_matrix;

    highp vec2 u_texsize;
    highp vec2 u_texsize_icon;

    highp float u_gamma_scale;
    bool u_rotate_symbol;
    highp vec2 u_pad1;
};

layout (std140) uniform SymbolDynamicUBO {
    highp float u_fade_change;
    highp float u_camera_to_center_distance;
    highp float u_device_pixel_ratio;
    highp float u_aspect_ratio;
};

layout (std140) uniform SymbolDrawablePaintUBO {
    highp vec4 u_fill_color;
    highp vec4 u_halo_color;
    highp float u_opacity;
    highp float u_halo_width;
    highp float u_halo_blur;
    highp float u_padding;
};

layout (std140) uniform SymbolDrawableTilePropsUBO {
    bool u_is_text;
    bool u_is_halo;
    bool u_pitch_with_map;
    bool u_is_size_zoom_constant;
    bool u_is_size_feature_constant;
    highp float u_size_t; // used to interpolate between zoom stops when size is a composite function
    highp float u_size; // used when size is both zoom and feature constant
    bool u_pad3;
};

layout (std140) uniform SymbolDrawableInterpolateUBO {
    highp float u_fill_color_t;
    highp float u_halo_color_t;
    highp float u_opacity_t;
    highp float u_halo_width_t;
    highp float u_halo_blur_t;
    highp float u_pad4,u_pad5,u_pad6;
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

    float EDGE_GAMMA = 0.105 / u_device_pixel_ratio;

    float gamma_scale = v_data1.x;
    float size = v_data1.y;

    float fontScale = size / 24.0;

    lowp vec4 color = fill_color;
    highp float gamma = EDGE_GAMMA / (fontScale * u_gamma_scale);
    lowp float buff = (256.0 - 64.0) / 256.0;
    if (u_is_halo) {
        color = halo_color;
        gamma = (halo_blur * 1.19 / SDF_PX + EDGE_GAMMA) / (fontScale * u_gamma_scale);
        buff = (6.0 - halo_width / fontScale) / SDF_PX;
    }

    lowp float dist = texture(u_texture, tex).a;
    highp float gamma_scaled = gamma * gamma_scale;
    highp float alpha = smoothstep(buff - gamma_scaled, buff + gamma_scaled, dist);

    fragColor = color * (alpha * opacity * fade_opacity);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
