#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "SymbolIconShader";

    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<AttributeInfo, 6> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
        
layout(location = 0) in ivec4 in_pos_offset;
layout(location = 1) in uvec4 in_data;
layout(location = 2) in ivec4 in_pixeloffset;
layout(location = 3) in vec3 in_projected_pos;
layout(location = 4) in float in_fade_opacity;

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) in vec2 in_opacity;
#endif    

layout(set = 0, binding = 1) uniform SymbolDrawableUBO {
    mat4 matrix;
    mat4 label_plane_matrix;
    mat4 coord_matrix;
    vec2 texsize;
    vec2 texsize_icon;
    float gamma_scale;
	bool rotate_symbol;
    vec2 pad;
} drawable;

layout(set = 0, binding = 2) uniform SymbolTilePropsUBO {
    bool is_text;
    bool is_halo;
    bool pitch_with_map;
    bool is_size_zoom_constant;
    bool is_size_feature_constant;
    float size_t;
    float size;
    float padding;
} tile;

layout(set = 0, binding = 3) uniform SymbolInterpolateUBO {
    float fill_color_t;
    float halo_color_t;
    float opacity_t;
    float halo_width_t;
    float halo_blur_t;
    float pad1, pad2, pad3;
} interp;

layout(location = 0) out mediump vec2 frag_tex;
layout(location = 1) out mediump float frag_opacity;

void main() {

    const vec2 a_pos = in_pos_offset.xy;
    const vec2 a_offset = in_pos_offset.zw;

    const vec2 a_tex = in_data.xy;
    const vec2 a_size = in_data.zw;

    const float a_size_min = floor(a_size[0] * 0.5);
    const vec2 a_pxoffset = in_pixeloffset.xy;
    const vec2 a_minFontScale = in_pixeloffset.zw / 256.0;

    const float segment_angle = -in_projected_pos[2];

    float size;
    if (!tile.is_size_zoom_constant && !tile.is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], tile.size_t) / 128.0;
    } else if (tile.is_size_zoom_constant && !tile.is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = tile.size;
    }

    const vec4 projectedPoint = drawable.matrix * vec4(a_pos, 0, 1);
    const float camera_to_anchor_distance = projectedPoint.w;
    // See comments in symbol_sdf.vertex
    const float distance_ratio = tile.pitch_with_map ?
        camera_to_anchor_distance / global.camera_to_center_distance :
        global.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
            0.5 + 0.5 * distance_ratio,
            0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
            4.0);

    size *= perspective_ratio;

    const float fontScale = tile.is_text ? size / 24.0 : size;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // See comments in symbol_sdf.vertex
        const vec4 offsetProjectedPoint = drawable.matrix * vec4(a_pos + vec2(1, 0), 0, 1);

        const vec2 a = projectedPoint.xy / projectedPoint.w;
        const vec2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;
        symbol_rotation = atan((b.y - a.y) / global.aspect_ratio, b.x - a.x);
    }

    const float angle_sin = sin(segment_angle + symbol_rotation);
    const float angle_cos = cos(segment_angle + symbol_rotation);
    const mat2 rotation_matrix = mat2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    const vec4 projected_pos = drawable.label_plane_matrix * vec4(in_projected_pos.xy, 0.0, 1.0);
    const vec2 pos0 = projected_pos.xy / projected_pos.w;
    const vec2 posOffset = a_offset * max(a_minFontScale, fontScale) / 32.0 + a_pxoffset / 16.0;
    gl_Position = drawable.coord_matrix * vec4(pos0 + rotation_matrix * posOffset, 0.0, 1.0);
    gl_Position.y *= -1.0;
    
    const vec2 raw_fade_opacity = unpack_opacity(in_fade_opacity);
    const float fade_change = raw_fade_opacity[1] > 0.5 ? global.symbol_fade_change : -global.symbol_fade_change;
    const float fade_opacity = max(0.0, min(1.0, raw_fade_opacity[0] + fade_change));

    frag_tex = a_tex / drawable.texsize;

#if defined(HAS_UNIFORM_u_opacity)
    frag_opacity = fade_opacity;
#else
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t) * fade_opacity;
#endif
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in mediump vec2 frag_tex;
layout(location = 1) in mediump float frag_opacity;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 2) uniform SymbolTilePropsUBO {
    bool is_text;
    bool is_halo;
    bool pitch_with_map;
    bool is_size_zoom_constant;
    bool is_size_feature_constant;
    float size_t;
    float size;
    float padding;
} tile;

layout(set = 0, binding = 4) uniform SymbolEvaluatedPropsUBO {
    vec4 text_fill_color;
    vec4 text_halo_color;
    float text_opacity;
    float text_halo_width;
    float text_halo_blur;
    float pad1;
    vec4 icon_fill_color;
    vec4 icon_halo_color;
    float icon_opacity;
    float icon_halo_width;
    float icon_halo_blur;
    float pad2;
} props;

layout(set = 1, binding = 0) uniform sampler2D image0_sampler;

void main() {
#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = (tile.is_text ? props.text_opacity : props.icon_opacity) * frag_opacity;
#else
    const float opacity = frag_opacity; // fade_opacity is baked in for this case
#endif

    out_color = texture(image0_sampler, frag_tex) * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::SymbolSDFIconShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "SymbolSDFIconShader";

    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<AttributeInfo, 10> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
        
layout(location = 0) in ivec4 in_pos_offset;
layout(location = 1) in uvec4 in_data;
layout(location = 2) in ivec4 in_pixeloffset;
layout(location = 3) in vec3 in_projected_pos;
layout(location = 4) in float in_fade_opacity;

#if !defined(HAS_UNIFORM_u_fill_color)
layout(location = 5) in vec4 in_fill_color;
#endif

#if !defined(HAS_UNIFORM_u_halo_color)
layout(location = 6) in vec4 in_halo_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 7) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_halo_width)
layout(location = 8) in vec2 in_halo_width;
#endif

#if !defined(HAS_UNIFORM_u_halo_blur)
layout(location = 9) in vec2 in_halo_blur;
#endif  

layout(set = 0, binding = 1) uniform SymbolDrawableUBO {
    mat4 matrix;
    mat4 label_plane_matrix;
    mat4 coord_matrix;
    vec2 texsize;
    vec2 texsize_icon;
    float gamma_scale;
	bool rotate_symbol;
    vec2 pad;
} drawable;

layout(set = 0, binding = 2) uniform SymbolTilePropsUBO {
    bool is_text;
    bool is_halo;
    bool pitch_with_map;
    bool is_size_zoom_constant;
    bool is_size_feature_constant;
    float size_t;
    float size;
    float padding;
} tile;

layout(set = 0, binding = 3) uniform SymbolInterpolateUBO {
    float fill_color_t;
    float halo_color_t;
    float opacity_t;
    float halo_width_t;
    float halo_blur_t;
    float pad1, pad2, pad3;
} interp;

layout(location = 0) out mediump vec2 frag_tex;
layout(location = 1) out mediump float frag_fade_opacity;
layout(location = 2) out mediump float frag_font_scale;
layout(location = 3) out mediump float frag_gamma_scale;

#if !defined(HAS_UNIFORM_u_fill_color)
layout(location = 4) out mediump vec4 frag_fill_color;
#endif

#if !defined(HAS_UNIFORM_u_halo_color)
layout(location = 5) out mediump vec4 frag_halo_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 6) out mediump float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_halo_width)
layout(location = 7) out mediump float frag_halo_width;
#endif

#if !defined(HAS_UNIFORM_u_halo_blur)
layout(location = 8) out mediump float frag_halo_blur;
#endif

void main() {

    const vec2 a_pos = in_pos_offset.xy;
    const vec2 a_offset = in_pos_offset.zw;

    const vec2 a_tex = in_data.xy;
    const vec2 a_size = in_data.zw;

    const float a_size_min = floor(a_size[0] * 0.5);
    const vec2 a_pxoffset = in_pixeloffset.xy;
    const float segment_angle = -in_projected_pos[2];

    float size;
    if (!tile.is_size_zoom_constant && !tile.is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], tile.size_t) / 128.0;
    } else if (tile.is_size_zoom_constant && !tile.is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = tile.size;
    }

    const vec4 projectedPoint = drawable.matrix * vec4(a_pos, 0, 1);
    const float camera_to_anchor_distance = projectedPoint.w;
    // If the label is pitched with the map, layout is done in pitched space,
    // which makes labels in the distance smaller relative to viewport space.
    // We counteract part of that effect by multiplying by the perspective ratio.
    // If the label isn't pitched with the map, we do layout in viewport space,
    // which makes labels in the distance larger relative to the features around
    // them. We counteract part of that effect by dividing by the perspective ratio.
    const float distance_ratio = tile.pitch_with_map ?
        camera_to_anchor_distance / global.camera_to_center_distance :
        global.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
            0.5 + 0.5 * distance_ratio,
            0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
            4.0);

    size *= perspective_ratio;

    const float fontScale = tile.is_text ? size / 24.0 : size;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // Point labels with 'rotation-alignment: map' are horizontal with respect to tile units
        // To figure out that angle in projected space, we draw a short horizontal line in tile
        // space, project it, and measure its angle in projected space.
        const vec4 offsetProjectedPoint = drawable.matrix * vec4(a_pos + vec2(1, 0), 0, 1);

        const vec2 a = projectedPoint.xy / projectedPoint.w;
        const vec2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;
        symbol_rotation = atan((b.y - a.y) / global.aspect_ratio, b.x - a.x);
    }

    const float angle_sin = sin(segment_angle + symbol_rotation);
    const float angle_cos = cos(segment_angle + symbol_rotation);
    const mat2 rotation_matrix = mat2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    const vec4 projected_pos = drawable.label_plane_matrix * vec4(in_projected_pos.xy, 0.0, 1.0);
    const vec2 pos_rot = a_offset / 32.0 * fontScale + a_pxoffset;
    const vec2 pos0 = projected_pos.xy / projected_pos.w + rotation_matrix * pos_rot;
    gl_Position = drawable.coord_matrix * vec4(pos0, 0.0, 1.0);
    gl_Position.y *= -1.0;
    
    const vec2 raw_fade_opacity = unpack_opacity(in_fade_opacity);
    const float fade_change = raw_fade_opacity[1] > 0.5 ? global.symbol_fade_change : -global.symbol_fade_change;

    frag_tex = a_tex / drawable.texsize;
    frag_fade_opacity = max(0.0, min(1.0, raw_fade_opacity[0] + fade_change));
    frag_font_scale = fontScale;
    frag_gamma_scale = gl_Position.w;

#if !defined(HAS_UNIFORM_u_fill_color)
    frag_fill_color = unpack_mix_color(in_fill_color, interp.fill_color_t);
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
    frag_halo_color = unpack_mix_color(in_halo_color, interp.halo_color_t);
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
    frag_halo_width = unpack_mix_float(in_halo_width, interp.halo_width_t);
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
    frag_halo_blur = unpack_mix_float(in_halo_blur, interp.halo_blur_t);
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in mediump vec2 frag_tex;
layout(location = 1) in mediump float frag_fade_opacity;
layout(location = 2) in mediump float frag_font_scale;
layout(location = 3) in mediump float frag_gamma_scale;

#if !defined(HAS_UNIFORM_u_fill_color)
layout(location = 4) in mediump vec4 frag_fill_color;
#endif

#if !defined(HAS_UNIFORM_u_halo_color)
layout(location = 5) in mediump vec4 frag_halo_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 6) in mediump float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_halo_width)
layout(location = 7) in mediump float frag_halo_width;
#endif

#if !defined(HAS_UNIFORM_u_halo_blur)
layout(location = 8) in mediump float frag_halo_blur;
#endif

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform SymbolDrawableUBO {
    mat4 matrix;
    mat4 label_plane_matrix;
    mat4 coord_matrix;
    vec2 texsize;
    vec2 texsize_icon;
    float gamma_scale;
	bool rotate_symbol;
    vec2 pad;
} drawable;

layout(set = 0, binding = 2) uniform SymbolTilePropsUBO {
    bool is_text;
    bool is_halo;
    bool pitch_with_map;
    bool is_size_zoom_constant;
    bool is_size_feature_constant;
    float size_t;
    float size;
    float padding;
} tile;

layout(set = 0, binding = 4) uniform SymbolEvaluatedPropsUBO {
    vec4 text_fill_color;
    vec4 text_halo_color;
    float text_opacity;
    float text_halo_width;
    float text_halo_blur;
    float pad1;
    vec4 icon_fill_color;
    vec4 icon_halo_color;
    float icon_opacity;
    float icon_halo_width;
    float icon_halo_blur;
    float pad2;
} props;

layout(set = 1, binding = 0) uniform sampler2D image0_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_fill_color)
    const vec4 fill_color = tile.is_text ? props.text_fill_color : props.icon_fill_color;
#else
    const vec4 fill_color = frag_fill_color;
#endif
#if defined(HAS_UNIFORM_u_halo_color)
    const vec4 halo_color = tile.is_text ? props.text_halo_color : props.icon_halo_color;
#else
    const vec4 halo_color = frag_halo_color;
#endif
#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = tile.is_text ? props.text_opacity : props.icon_opacity;
#else
    const float opacity = frag_opacity;
#endif
#if defined(HAS_UNIFORM_u_halo_width)
    const float halo_width = tile.is_text ? props.text_halo_width : props.icon_halo_width;
#else
    const float halo_width = frag_halo_width;
#endif
#if defined(HAS_UNIFORM_u_halo_blur)
    const float halo_blur = tile.is_text ? props.text_halo_blur : props.icon_halo_blur;
#else
    const float halo_blur = frag_halo_blur;
#endif

    const float EDGE_GAMMA = 0.105 / DEVICE_PIXEL_RATIO;
    const float fontGamma = frag_font_scale * drawable.gamma_scale;
    const vec4 color = tile.is_halo ? halo_color : fill_color;
    const float gamma = ((tile.is_halo ? (halo_blur * 1.19 / SDF_PX) : 0) + EDGE_GAMMA) / fontGamma;
    const float buff = tile.is_halo ? (6.0 - halo_width / frag_font_scale) / SDF_PX : (256.0 - 64.0) / 256.0;
    const float dist = texture(image0_sampler, frag_tex).a;
    const float gamma_scaled = gamma * frag_gamma_scale;
    const float alpha = smoothstep(buff - gamma_scaled, buff + gamma_scaled, dist);

    out_color = color * (alpha * opacity * frag_fade_opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "SymbolTextAndIconShader";

    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto vertex = R"(
        
#define SDF 1.0
#define ICON 0.0

layout(location = 0) in ivec4 in_pos_offset;
layout(location = 1) in uvec4 in_data;
layout(location = 2) in vec3 in_projected_pos;
layout(location = 3) in float in_fade_opacity;

#if !defined(HAS_UNIFORM_u_fill_color)
layout(location = 4) in vec4 in_fill_color;
#endif

#if !defined(HAS_UNIFORM_u_halo_color)
layout(location = 5) in vec4 in_halo_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 6) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_halo_width)
layout(location = 7) in vec2 in_halo_width;
#endif

#if !defined(HAS_UNIFORM_u_halo_blur)
layout(location = 8) in vec2 in_halo_blur;
#endif  

layout(set = 0, binding = 1) uniform SymbolDrawableUBO {
    mat4 matrix;
    mat4 label_plane_matrix;
    mat4 coord_matrix;
    vec2 texsize;
    vec2 texsize_icon;
    float gamma_scale;
	bool rotate_symbol;
    vec2 pad;
} drawable;

layout(set = 0, binding = 2) uniform SymbolTilePropsUBO {
    bool is_text;
    bool is_halo;
    bool pitch_with_map;
    bool is_size_zoom_constant;
    bool is_size_feature_constant;
    float size_t;
    float size;
    float padding;
} tile;

layout(set = 0, binding = 3) uniform SymbolInterpolateUBO {
    float fill_color_t;
    float halo_color_t;
    float opacity_t;
    float halo_width_t;
    float halo_blur_t;
    float pad1, pad2, pad3;
} interp;

layout(location = 0) out mediump vec2 frag_tex;
layout(location = 1) out mediump float frag_fade_opacity;
layout(location = 2) out mediump float frag_font_scale;
layout(location = 3) out mediump float frag_gamma_scale;

#if !defined(HAS_UNIFORM_u_fill_color)
layout(location = 4) out mediump vec4 frag_fill_color;
#endif

#if !defined(HAS_UNIFORM_u_halo_color)
layout(location = 5) out mediump vec4 frag_halo_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 6) out mediump float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_halo_width)
layout(location = 7) out mediump float frag_halo_width;
#endif

#if !defined(HAS_UNIFORM_u_halo_blur)
layout(location = 8) out mediump float frag_halo_blur;
#endif

layout(location = 9) out int frag_is_icon;

void main() {

    const vec2 a_pos = in_pos_offset.xy;
    const vec2 a_offset = in_pos_offset.zw;

    const vec2 a_tex = in_data.xy;
    const vec2 a_size = in_data.zw;

    const float a_size_min = floor(a_size[0] * 0.5);
    const float is_sdf = a_size[0] - 2.0 * a_size_min;
    const float segment_angle = -in_projected_pos[2];

    float size;
    if (!tile.is_size_zoom_constant && !tile.is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], tile.size_t) / 128.0;
    } else if (tile.is_size_zoom_constant && !tile.is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = tile.size;
    }

    const vec4 projectedPoint = drawable.matrix * vec4(a_pos, 0, 1);
    const float camera_to_anchor_distance = projectedPoint.w;
    // If the label is pitched with the map, layout is done in pitched space,
    // which makes labels in the distance smaller relative to viewport space.
    // We counteract part of that effect by multiplying by the perspective ratio.
    // If the label isn't pitched with the map, we do layout in viewport space,
    // which makes labels in the distance larger relative to the features around
    // them. We counteract part of that effect by dividing by the perspective ratio.
    const float distance_ratio = tile.pitch_with_map ?
        camera_to_anchor_distance / global.camera_to_center_distance :
        global.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
            0.5 + 0.5 * distance_ratio,
            0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
            4.0);

    size *= perspective_ratio;

    const float fontScale = size / 24.0;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // Point labels with 'rotation-alignment: map' are horizontal with respect to tile units
        // To figure out that angle in projected space, we draw a short horizontal line in tile
        // space, project it, and measure its angle in projected space.
        const vec4 offsetProjectedPoint = drawable.matrix * vec4(a_pos + vec2(1, 0), 0, 1);

        const vec2 a = projectedPoint.xy / projectedPoint.w;
        const vec2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;
        symbol_rotation = atan((b.y - a.y) / global.aspect_ratio, b.x - a.x);
    }

    const float angle_sin = sin(segment_angle + symbol_rotation);
    const float angle_cos = cos(segment_angle + symbol_rotation);
    const mat2 rotation_matrix = mat2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    const vec4 projected_pos = drawable.label_plane_matrix * vec4(in_projected_pos.xy, 0.0, 1.0);
    const vec2 pos_rot = a_offset / 32.0 * fontScale;
    const vec2 pos0 = projected_pos.xy / projected_pos.w + rotation_matrix * pos_rot;
    gl_Position = drawable.coord_matrix * vec4(pos0, 0.0, 1.0);
    gl_Position.y *= -1.0;
    
    const vec2 raw_fade_opacity = unpack_opacity(in_fade_opacity);
    const float fade_change = raw_fade_opacity[1] > 0.5 ? global.symbol_fade_change : -global.symbol_fade_change;

    const bool is_icon = (is_sdf == ICON);
	frag_is_icon = int(is_icon);
    
	frag_tex = a_tex / (is_icon ? drawable.texsize_icon : drawable.texsize);
    frag_fade_opacity = max(0.0, min(1.0, raw_fade_opacity[0] + fade_change));
    frag_font_scale = fontScale;
    frag_gamma_scale = gl_Position.w;

#if !defined(HAS_UNIFORM_u_fill_color)
    frag_fill_color = unpack_mix_color(in_fill_color, interp.fill_color_t);
#endif
#if !defined(HAS_UNIFORM_u_halo_color)
    frag_halo_color = unpack_mix_color(in_halo_color, interp.halo_color_t);
#endif
#if !defined(HAS_UNIFORM_u_halo_width)
    frag_halo_width = unpack_mix_float(in_halo_width, interp.halo_width_t);
#endif
#if !defined(HAS_UNIFORM_u_halo_blur)
    frag_halo_blur = unpack_mix_float(in_halo_blur, interp.halo_blur_t);
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in mediump vec2 frag_tex;
layout(location = 1) in mediump float frag_fade_opacity;
layout(location = 2) in mediump float frag_font_scale;
layout(location = 3) in mediump float frag_gamma_scale;

#if !defined(HAS_UNIFORM_u_fill_color)
layout(location = 4) in mediump vec4 frag_fill_color;
#endif

#if !defined(HAS_UNIFORM_u_halo_color)
layout(location = 5) in mediump vec4 frag_halo_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 6) in mediump float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_halo_width)
layout(location = 7) in mediump float frag_halo_width;
#endif

#if !defined(HAS_UNIFORM_u_halo_blur)
layout(location = 8) in mediump float frag_halo_blur;
#endif

layout(location = 9) flat in int frag_is_icon;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform SymbolDrawableUBO {
    mat4 matrix;
    mat4 label_plane_matrix;
    mat4 coord_matrix;
    vec2 texsize;
    vec2 texsize_icon;
    float gamma_scale;
	bool rotate_symbol;
    vec2 pad;
} drawable;

layout(set = 0, binding = 2) uniform SymbolTilePropsUBO {
    bool is_text;
    bool is_halo;
    bool pitch_with_map;
    bool is_size_zoom_constant;
    bool is_size_feature_constant;
    float size_t;
    float size;
    float padding;
} tile;

layout(set = 0, binding = 4) uniform SymbolEvaluatedPropsUBO {
    vec4 text_fill_color;
    vec4 text_halo_color;
    float text_opacity;
    float text_halo_width;
    float text_halo_blur;
    float pad1;
    vec4 icon_fill_color;
    vec4 icon_halo_color;
    float icon_opacity;
    float icon_halo_width;
    float icon_halo_blur;
    float pad2;
} props;

layout(set = 1, binding = 0) uniform sampler2D glyph_image;
layout(set = 1, binding = 1) uniform sampler2D icon_image;

void main() {
#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_fill_color)
    const vec4 fill_color = tile.is_text ? props.text_fill_color : props.icon_fill_color;
#else
    const vec4 fill_color = frag_fill_color;
#endif
#if defined(HAS_UNIFORM_u_halo_color)
    const vec4 halo_color = tile.is_text ? props.text_halo_color : props.icon_halo_color;
#else
    const vec4 halo_color = frag_halo_color;
#endif
#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = tile.is_text ? props.text_opacity : props.icon_opacity;
#else
    const float opacity = frag_opacity;
#endif
#if defined(HAS_UNIFORM_u_halo_width)
    const float halo_width = tile.is_text ? props.text_halo_width : props.icon_halo_width;
#else
    const float halo_width = frag_halo_width;
#endif
#if defined(HAS_UNIFORM_u_halo_blur)
    const float halo_blur = tile.is_text ? props.text_halo_blur : props.icon_halo_blur;
#else
    const float halo_blur = frag_halo_blur;
#endif

    if (bool(frag_is_icon)) {
        const float alpha = opacity * frag_fade_opacity;
        out_color = texture(icon_image, frag_tex) * alpha;
        return;
    }

    const float EDGE_GAMMA = 0.105 / DEVICE_PIXEL_RATIO;
    const float fontGamma = frag_font_scale * drawable.gamma_scale;
    const vec4 color = tile.is_halo ? halo_color : fill_color;
    const float gamma = ((tile.is_halo ? (halo_blur * 1.19 / SDF_PX) : 0) + EDGE_GAMMA) / fontGamma;
    const float buff = tile.is_halo ? (6.0 - halo_width / frag_font_scale) / SDF_PX : (256.0 - 64.0) / 256.0;
    const float dist = texture(glyph_image, frag_tex).a;
    const float gamma_scaled = gamma * frag_gamma_scale;
    const float alpha = smoothstep(buff - gamma_scaled, buff + gamma_scaled, dist);

    out_color = color * (alpha * opacity * frag_fade_opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "CustomSymbolIconShader";

    static const std::array<UniformBlockInfo, 2> uniforms;
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_tex;

layout(set = 0, binding = 1) uniform CustomSymbolIconDrawableUBO {
    mat4 matrix;
} drawable;

layout(set = 0, binding = 2) uniform CustomSymbolIconParametersUBO {
    vec2 extrude_scale;
    vec2 anchor;
    float angle_degrees;
    bool scale_with_map;
    bool pitch_with_map;
    float camera_to_center_distance;
    float aspect_ratio;
    float pad0, pad1, pad3;
} parameters;

layout(location = 0) out vec2 frag_tex;

vec2 rotateVec2(vec2 v, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    return vec2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

vec2 ellipseRotateVec2(vec2 v, float angle, float radiusRatio /* A/B */) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    float invRatio = 1.0 / radiusRatio;
    return vec2(v.x * cosA - radiusRatio * v.y * sinA, invRatio * v.x * sinA + v.y * cosA);
}

void main() {
    const vec2 extrude = mod(in_position, 2.0) * 2.0 - 1.0;
    const vec2 anchor = (parameters.anchor - vec2(0.5, 0.5)) * 2.0;
    const vec2 center = floor(in_position * 0.5);
    const float angle = radians(-parameters.angle_degrees);
    vec2 corner = extrude - anchor;

    vec4 position;
    if (parameters.pitch_with_map) {
        if (parameters.scale_with_map) {
            corner *= parameters.extrude_scale;
        } else {
            vec4 projected_center = drawable.matrix * vec4(center, 0, 1);
            corner *= parameters.extrude_scale * (projected_center.w / parameters.camera_to_center_distance);
        }
        corner = center + rotateVec2(corner, angle);
        position = drawable.matrix * vec4(corner, 0, 1);
    } else {
        position = drawable.matrix * vec4(center, 0, 1);
        const float factor = parameters.scale_with_map ? parameters.camera_to_center_distance : position.w;
        position.xy += ellipseRotateVec2(corner * parameters.extrude_scale * factor, angle, parameters.aspect_ratio);
    }

    gl_Position = position;
    gl_Position.y *= -1.0;

    frag_tex = in_tex;
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) in vec2 frag_tex;
layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 0) uniform sampler2D image_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    out_color = texture(image_sampler, frag_tex);
}
)";
};

} // namespace shaders
} // namespace mbgl
