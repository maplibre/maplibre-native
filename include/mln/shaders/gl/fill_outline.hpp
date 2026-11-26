// Generated code, do not modify this file!
#pragma once
#include <mln/shaders/shader_source.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "FillOutlineShader";
    static constexpr const char* vertex = R"(layout (std140) uniform GlobalPaintParamsUBO {
    highp vec2 u_pattern_atlas_texsize;
    highp vec2 u_units_to_pixels;
    highp vec2 u_world_size;
    highp float u_camera_to_center_distance;
    highp float u_symbol_fade_change;
    highp float u_aspect_ratio;
    highp float u_pixel_ratio;
    highp float u_map_zoom;
    lowp float global_pad1;
};

layout (std140) uniform FillOutlineDrawableUBO {
    highp mat4 u_matrix;
    // Interpolations
    highp float u_outline_color_t;
    highp float u_opacity_t;
    lowp float u_fade_zoom_t;
    lowp float drawable_pad2;
};

layout (std140) uniform FillEvaluatedPropsUBO {
    highp vec4 u_color;
    highp vec4 u_outline_color;
    highp float u_opacity;
    highp float u_rounded_corner_distance;
    highp float u_fade;
    highp float u_from_scale;
    highp float u_to_scale;
    highp float u_fade_zoom;
    lowp float props_pad2;
    lowp float props_pad3;
};

layout (std140) uniform FillFadeZoomUBO {
    highp vec4 u_fade_zoom_table[104];
};

out lowp float v_fade_zoom_opacity;

layout (location = 0) in vec2 a_pos;

out vec2 v_pos;

#ifndef HAS_UNIFORM_u_outline_color
layout (location = 1) in highp vec4 a_outline_color;
out highp vec4 outline_color;
#endif
#ifndef HAS_UNIFORM_u_opacity
layout (location = 2) in lowp vec2 a_opacity;
out lowp float opacity;
#endif
#ifndef HAS_UNIFORM_u_fade_zoom
layout (location = 3) in highp vec2 a_fade_zoom;
#endif

void main() {
    #ifndef HAS_UNIFORM_u_outline_color
outline_color = unpack_mix_color(a_outline_color, u_outline_color_t);
#else
highp vec4 outline_color = u_outline_color;
#endif
    #ifndef HAS_UNIFORM_u_opacity
opacity = unpack_mix_vec2(a_opacity, u_opacity_t);
#else
lowp float opacity = u_opacity;
#endif
    #ifndef HAS_UNIFORM_u_fade_zoom
highp float fade_zoom = unpack_mix_vec2(a_fade_zoom, u_fade_zoom_t);
#else
highp float fade_zoom = u_fade_zoom;
#endif

    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    v_pos = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * u_world_size;

    // Fade opacity for this fade zoom: one table entry per 1/16 of a zoom level, packed four to a vec4.
    int fade_zoom_index = clamp(int(floor(fade_zoom * 16.0 + 0.5)), 0, 415);
    v_fade_zoom_opacity = u_fade_zoom_table[fade_zoom_index / 4][fade_zoom_index % 4];
}
)";
    static constexpr const char* fragment = R"(layout (std140) uniform FillEvaluatedPropsUBO {
    highp vec4 u_color;
    highp vec4 u_outline_color;
    highp float u_opacity;
    highp float u_rounded_corner_distance;
    highp float u_fade;
    highp float u_from_scale;
    highp float u_to_scale;
    highp float u_fade_zoom;
    lowp float props_pad2;
    lowp float props_pad3;
};

in lowp float v_fade_zoom_opacity;

in vec2 v_pos;

#ifndef HAS_UNIFORM_u_outline_color
in highp vec4 outline_color;
#endif
#ifndef HAS_UNIFORM_u_opacity
in lowp float opacity;
#endif

void main() {
    #ifdef HAS_UNIFORM_u_outline_color
highp vec4 outline_color = u_outline_color;
#endif
    #ifdef HAS_UNIFORM_u_opacity
lowp float opacity = u_opacity;
#endif

    float dist = length(v_pos - gl_FragCoord.xy);
    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    fragColor = outline_color * (alpha * opacity);

    fragColor *= v_fade_zoom_opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mln
