// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "SymbolIconShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec4 a_pos_offset;
layout (location = 1) in vec4 a_data;
layout (location = 2) in vec4 a_pixeloffset;
layout (location = 3) in vec3 a_projected_pos;
layout (location = 4) in float a_fade_opacity;

layout (std140) uniform GlobalPaintParamsUBO {
    highp vec2 u_pattern_atlas_texsize;
    highp vec2 u_units_to_pixels;
    highp vec2 u_world_size;
    highp float u_camera_to_center_distance;
    highp float u_symbol_fade_change;
    highp float u_aspect_ratio;
    highp float u_pixel_ratio;
    highp float global_pad1, global_pad2;
};

layout (std140) uniform SymbolDrawableUBO {
    highp mat4 u_matrix;
    highp mat4 u_label_plane_matrix;
    highp mat4 u_coord_matrix;

    highp vec2 u_texsize;
    highp vec2 u_texsize_icon;

    highp float u_gamma_scale;
    bool u_rotate_symbol;
    highp vec2 drawable_pad1;
};

layout (std140) uniform SymbolTilePropsUBO {
    bool u_is_text;
    bool u_is_halo;
    bool u_pitch_with_map;
    bool u_is_size_zoom_constant;
    bool u_is_size_feature_constant;
    highp float u_size_t; // used to interpolate between zoom stops when size is a composite function
    highp float u_size; // used when size is both zoom and feature constant
    bool tileprops_pad1;
};

layout (std140) uniform SymbolInterpolateUBO {
    highp float u_fill_color_t;
    highp float u_halo_color_t;
    highp float u_opacity_t;
    highp float u_halo_width_t;
    highp float u_halo_blur_t;
    highp float interp_pad1, interp_pad2, interp_pad3;
};

layout (std140) uniform SymbolEvaluatedPropsUBO {
    highp vec4 u_text_fill_color;
    highp vec4 u_text_halo_color;
    highp float u_text_opacity;
    highp float u_text_halo_width;
    highp float u_text_halo_blur;
    highp float props_pad1;
    highp vec4 u_icon_fill_color;
    highp vec4 u_icon_halo_color;
    highp float u_icon_opacity;
    highp float u_icon_halo_width;
    highp float u_icon_halo_blur;
    highp float props_pad2;
};

out vec2 v_tex;
out float v_fade_opacity;

#ifndef HAS_UNIFORM_u_opacity
layout (location = 5) in lowp vec2 a_opacity;
out lowp float opacity;
#endif

void main() {
    highp float u_opacity = u_is_text ? u_text_opacity : u_icon_opacity;

    #ifndef HAS_UNIFORM_u_opacity
opacity = unpack_mix_vec2(a_opacity, u_opacity_t);
#else
lowp float opacity = u_opacity;
#endif

    vec2 a_pos = a_pos_offset.xy;
    vec2 a_offset = a_pos_offset.zw;

    vec2 a_tex = a_data.xy;
    vec2 a_size = a_data.zw;

    float a_size_min = floor(a_size[0] * 0.5);
    vec2 a_pxoffset = a_pixeloffset.xy;
    vec2 a_minFontScale = a_pixeloffset.zw / 256.0;

    highp float segment_angle = -a_projected_pos[2];
    float size;

    if (!u_is_size_zoom_constant && !u_is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], u_size_t) / 128.0;
    } else if (u_is_size_zoom_constant && !u_is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = u_size;
    }

    vec4 projectedPoint = u_matrix * vec4(a_pos, 0, 1);
    highp float camera_to_anchor_distance = projectedPoint.w;
    // See comments in symbol_sdf.vertex
    highp float distance_ratio = u_pitch_with_map ?
        camera_to_anchor_distance / u_camera_to_center_distance :
        u_camera_to_center_distance / camera_to_anchor_distance;
    highp float perspective_ratio = clamp(
            0.5 + 0.5 * distance_ratio,
            0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
            4.0);

    size *= perspective_ratio;

    float fontScale = u_is_text ? size / 24.0 : size;

    highp float symbol_rotation = 0.0;
    if (u_rotate_symbol) {
        // See comments in symbol_sdf.vertex
        vec4 offsetProjectedPoint = u_matrix * vec4(a_pos + vec2(1, 0), 0, 1);

        vec2 a = projectedPoint.xy / projectedPoint.w;
        vec2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;

        symbol_rotation = atan((b.y - a.y) / u_aspect_ratio, b.x - a.x);
    }

    highp float angle_sin = sin(segment_angle + symbol_rotation);
    highp float angle_cos = cos(segment_angle + symbol_rotation);
    mat2 rotation_matrix = mat2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    vec4 projected_pos = u_label_plane_matrix * vec4(a_projected_pos.xy, 0.0, 1.0);
    gl_Position = u_coord_matrix * vec4(projected_pos.xy / projected_pos.w + rotation_matrix * (a_offset / 32.0 * max(a_minFontScale, fontScale) + a_pxoffset / 16.0), 0.0, 1.0);

    v_tex = a_tex / u_texsize;
    vec2 fade_opacity = unpack_opacity(a_fade_opacity);
    float fade_change = fade_opacity[1] > 0.5 ? u_symbol_fade_change : -u_symbol_fade_change;
    v_fade_opacity = max(0.0, min(1.0, fade_opacity[0] + fade_change));
}
)";
    static constexpr const char* fragment = R"(uniform sampler2D u_texture;

layout (std140) uniform SymbolTilePropsUBO {
    bool u_is_text;
    bool u_is_halo;
    bool u_pitch_with_map;
    bool u_is_size_zoom_constant;
    bool u_is_size_feature_constant;
    highp float u_size_t; // used to interpolate between zoom stops when size is a composite function
    highp float u_size; // used when size is both zoom and feature constant
    bool tileprops_pad1;
};

layout (std140) uniform SymbolEvaluatedPropsUBO {
    highp vec4 u_text_fill_color;
    highp vec4 u_text_halo_color;
    highp float u_text_opacity;
    highp float u_text_halo_width;
    highp float u_text_halo_blur;
    highp float props_pad1;
    highp vec4 u_icon_fill_color;
    highp vec4 u_icon_halo_color;
    highp float u_icon_opacity;
    highp float u_icon_halo_width;
    highp float u_icon_halo_blur;
    highp float props_pad2;
};

in vec2 v_tex;
in float v_fade_opacity;

#ifndef HAS_UNIFORM_u_opacity
in lowp float opacity;
#endif

void main() {
    highp float u_opacity = u_is_text ? u_text_opacity : u_icon_opacity;

    #ifdef HAS_UNIFORM_u_opacity
lowp float opacity = u_opacity;
#endif

    lowp float alpha = opacity * v_fade_opacity;
    fragColor = texture(u_texture, v_tex) * alpha;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
