#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

constexpr auto symbolShaderPrelude = R"(
// Symbol shader UBO indices
const idGlobalPaintParamsUBO = 0u;
const idGlobalUBOIndex = 1u;
const idSymbolDrawableUBO = 2u;
const idSymbolTilePropsUBO = 3u;
const idSymbolEvaluatedPropsUBO = 4u;
const symbolUBOCount = 5u;

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

struct SymbolDrawableUBO {
    matrix: mat4x4<f32>,
    label_plane_matrix: mat4x4<f32>,
    coord_matrix: mat4x4<f32>,

    texsize: vec2<f32>,
    texsize_icon: vec2<f32>,

    is_text_prop: u32,
    rotate_symbol: u32,
    pitch_with_map: u32,
    is_size_zoom_constant: u32,
    is_size_feature_constant: u32,
    is_offset: u32,

    size_t: f32,
    size: f32,

    // Interpolations
    fill_color_t: f32,
    halo_color_t: f32,
    opacity_t: f32,
    halo_width_t: f32,
    halo_blur_t: f32,
};

struct SymbolTilePropsUBO {
    is_text: u32,
    is_halo: u32,
    gamma_scale: f32,
    pad1: f32,
};

struct SymbolEvaluatedPropsUBO {
    text_fill_color: vec4<f32>,
    text_halo_color: vec4<f32>,
    text_opacity: f32,
    text_halo_width: f32,
    text_halo_blur: f32,
    pad1: f32,
    icon_fill_color: vec4<f32>,
    icon_halo_color: vec4<f32>,
    icon_opacity: f32,
    icon_halo_width: f32,
    icon_halo_blur: f32,
    pad2: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

const c_offscreen_degenerate_triangle_location: f32 = -2.0;
)";

template <>
struct ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "SymbolIconShader";
    static const std::array<AttributeInfo, 6> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    static constexpr auto prelude = symbolShaderPrelude;

    static constexpr auto vertex = R"()"
                                   R"(
struct VertexInput {
    @location(3) pos_offset: vec4<i32>,
    @location(4) data: vec4<u32>,
    @location(5) pixeloffset: vec4<i32>,
    @location(6) projected_pos: vec3<f32>,
    @location(7) fade_opacity: f32,
#ifndef HAS_UNIFORM_u_opacity
    @location(8) opacity: f32,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
#ifndef HAS_UNIFORM_u_opacity
    @location(2) opacity: f32,
#endif
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<SymbolDrawableUBO>;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let drawable = drawableVector[globalIndex.value];

    let raw_fade_opacity = unpack_opacity(in.fade_opacity);
    let fade_change = select(-paintParams.symbol_fade_change, paintParams.symbol_fade_change, raw_fade_opacity.y > 0.5);
    let fade_opacity = max(0.0, min(1.0, raw_fade_opacity.x + fade_change));

#ifdef HAS_UNIFORM_u_opacity
    let final_opacity = fade_opacity;
#else
    let paint_opacity = unpack_mix_float(vec2<f32>(in.opacity, in.opacity), drawable.opacity_t);
    let final_opacity = paint_opacity * fade_opacity;
#endif

    // Cull vertices with zero opacity
    if (final_opacity == 0.0) {
        out.position = vec4<f32>(c_offscreen_degenerate_triangle_location,
                                 c_offscreen_degenerate_triangle_location,
                                 c_offscreen_degenerate_triangle_location, 1.0);
        return out;
    }

    let a_pos = vec2<f32>(f32(in.pos_offset.x), f32(in.pos_offset.y));
    let a_offset = vec2<f32>(f32(in.pos_offset.z), f32(in.pos_offset.w));

    let a_tex = vec2<f32>(f32(in.data.x), f32(in.data.y));
    let a_size = vec2<f32>(f32(in.data.z), f32(in.data.w));

    let a_size_min = floor(a_size.x * 0.5);
    let a_pxoffset = vec2<f32>(f32(in.pixeloffset.x), f32(in.pixeloffset.y));
    let a_minFontScale = vec2<f32>(f32(in.pixeloffset.z), f32(in.pixeloffset.w)) / 256.0;

    let segment_angle = -in.projected_pos.z;

    let size_zoom_constant = drawable.is_size_zoom_constant != 0u;
    let size_feature_constant = drawable.is_size_feature_constant != 0u;
    var size: f32;
    if (!size_zoom_constant && !size_feature_constant) {
        size = mix(a_size_min, a_size.y, drawable.size_t) / 128.0;
    } else if (size_zoom_constant && !size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = drawable.size;
    }

    let projectedPoint = drawable.matrix * vec4<f32>(a_pos, 0.0, 1.0);
    let camera_to_anchor_distance = projectedPoint.w;
    let pitch_with_map = drawable.pitch_with_map != 0u;
    let distance_ratio = select(
        paintParams.camera_to_center_distance / camera_to_anchor_distance,
        camera_to_anchor_distance / paintParams.camera_to_center_distance,
        pitch_with_map
    );
    let perspective_ratio = clamp(0.5 + 0.5 * distance_ratio, 0.0, 4.0);

    if (drawable.is_offset == 0u) {
        size *= perspective_ratio;
    }

    let is_text = drawable.is_text_prop != 0u;
    let fontScale = select(size, size / 24.0, is_text);

    var symbol_rotation = 0.0;
    if (drawable.rotate_symbol != 0u) {
        let offsetProjectedPoint = drawable.matrix * vec4<f32>(a_pos + vec2<f32>(1.0, 0.0), 0.0, 1.0);
        let a = projectedPoint.xy / projectedPoint.w;
        let b = offsetProjectedPoint.xy / offsetProjectedPoint.w;
        symbol_rotation = atan2((b.y - a.y) / paintParams.aspect_ratio, b.x - a.x);
    }

    let angle_sin = sin(segment_angle + symbol_rotation);
    let angle_cos = cos(segment_angle + symbol_rotation);
    let rotation_matrix = mat2x2<f32>(angle_cos, -angle_sin, angle_sin, angle_cos);

    let projected_pos = drawable.label_plane_matrix * vec4<f32>(in.projected_pos.xy, 0.0, 1.0);
    let pos0 = projected_pos.xy / projected_pos.w;
    let posOffset = a_offset * max(a_minFontScale, vec2<f32>(fontScale)) / 32.0 + a_pxoffset / 16.0;
    let position = drawable.coord_matrix * vec4<f32>(pos0 + rotation_matrix * posOffset, 0.0, 1.0);

    out.position = position;
    out.tex = a_tex / drawable.texsize;
    out.fade_opacity = fade_opacity;
#ifndef HAS_UNIFORM_u_opacity
    out.opacity = final_opacity;
#endif

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
#ifndef HAS_UNIFORM_u_opacity
    @location(2) opacity: f32,
#endif
};

@group(0) @binding(3) var<storage, read> tilePropsVector: array<SymbolTilePropsUBO>;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(4) var<uniform> props: SymbolEvaluatedPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var image: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];
#ifdef HAS_UNIFORM_u_opacity
    let base_opacity = select(props.icon_opacity, props.text_opacity, tileProps.is_text != 0u);
    let opacity = base_opacity * in.fade_opacity;
#else
    let opacity = select(props.icon_opacity, props.text_opacity, tileProps.is_text != 0u) * in.opacity;
#endif
    return textureSample(image, texture_sampler, in.tex) * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "SymbolSDFShader";
    static const std::array<AttributeInfo, 10> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    static constexpr auto prelude = symbolShaderPrelude;

    static constexpr auto vertex = R"()"
                                   R"(
struct VertexInput {
    @location(3) pos_offset: vec4<i32>,
    @location(4) data: vec4<u32>,
    @location(5) pixeloffset: vec4<i32>,
    @location(6) projected_pos: vec3<f32>,
    @location(7) fade_opacity: f32,
#ifndef HAS_UNIFORM_u_fill_color
    @location(8) fill_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_halo_color
    @location(9) halo_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(10) opacity: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_width
    @location(11) halo_width: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    @location(12) halo_blur: f32,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
    @location(2) gamma_scale: f32,
    @location(3) fontScale: f32,
#ifndef HAS_UNIFORM_u_fill_color
    @location(4) fill_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_halo_color
    @location(5) halo_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(6) opacity: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_width
    @location(7) halo_width: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    @location(8) halo_blur: f32,
#endif
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<SymbolDrawableUBO>;
@group(0) @binding(4) var<uniform> props: SymbolEvaluatedPropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let drawable = drawableVector[globalIndex.value];

    let fade_opacity = unpack_opacity(in.fade_opacity);
    let fade_change = select(-paintParams.symbol_fade_change, paintParams.symbol_fade_change, fade_opacity.y > 0.5);
    let fo = max(0.0, min(1.0, fade_opacity.x + fade_change));

    // Cull vertices with zero opacity
    if (fo == 0.0) {
        out.position = vec4<f32>(c_offscreen_degenerate_triangle_location,
                                 c_offscreen_degenerate_triangle_location,
                                 c_offscreen_degenerate_triangle_location, 1.0);
        return out;
    }

    let a_pos = vec2<f32>(f32(in.pos_offset.x), f32(in.pos_offset.y));
    let a_offset = vec2<f32>(f32(in.pos_offset.z), f32(in.pos_offset.w));

    let a_tex = vec2<f32>(f32(in.data.x), f32(in.data.y));
    let a_size = vec2<f32>(f32(in.data.z), f32(in.data.w));

    let a_size_min = floor(a_size.x * 0.5);
    let a_pxoffset = vec2<f32>(f32(in.pixeloffset.x), f32(in.pixeloffset.y));

    let segment_angle = -in.projected_pos.z;

    var size: f32;
    let size_zoom_constant = drawable.is_size_zoom_constant != 0u;
    let size_feature_constant = drawable.is_size_feature_constant != 0u;
    if (!size_zoom_constant && !size_feature_constant) {
        size = mix(a_size_min, a_size.y, drawable.size_t) / 128.0;
    } else if (size_zoom_constant && !size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = drawable.size;
    }

    let projectedPoint = drawable.matrix * vec4<f32>(a_pos, 0.0, 1.0);
    let camera_to_anchor_distance = projectedPoint.w;
    let pitch_with_map = drawable.pitch_with_map != 0u;
    let distance_ratio = select(
        paintParams.camera_to_center_distance / camera_to_anchor_distance,
        camera_to_anchor_distance / paintParams.camera_to_center_distance,
        pitch_with_map
    );
    let perspective_ratio = clamp(0.5 + 0.5 * distance_ratio, 0.0, 4.0);

    if (drawable.is_offset == 0u) {
        size *= perspective_ratio;
    }

    let is_text = drawable.is_text_prop != 0u;
    let fontScale = select(size, size / 24.0, is_text);

    var symbol_rotation = 0.0;
    if (drawable.rotate_symbol != 0u) {
        let offsetProjectedPoint = drawable.matrix * vec4<f32>(a_pos + vec2<f32>(1.0, 0.0), 0.0, 1.0);
        let a = projectedPoint.xy / projectedPoint.w;
        let b = offsetProjectedPoint.xy / offsetProjectedPoint.w;
        symbol_rotation = atan2((b.y - a.y) / paintParams.aspect_ratio, b.x - a.x);
    }

    let angle_sin = sin(segment_angle + symbol_rotation);
    let angle_cos = cos(segment_angle + symbol_rotation);
    let rotation_matrix = mat2x2<f32>(angle_cos, -angle_sin, angle_sin, angle_cos);

    let projected_pos = drawable.label_plane_matrix * vec4<f32>(in.projected_pos.xy, 0.0, 1.0);
    let pos_rot = a_offset / 32.0 * fontScale + a_pxoffset;
    let pos0 = projected_pos.xy / projected_pos.w + rotation_matrix * pos_rot;
    let position = drawable.coord_matrix * vec4<f32>(pos0, 0.0, 1.0);

    out.position = position;
    out.tex = a_tex / drawable.texsize;
    out.gamma_scale = position.w;
    out.fontScale = fontScale;
    out.fade_opacity = fo;
#ifndef HAS_UNIFORM_u_fill_color
    out.fill_color = unpack_mix_color(in.fill_color, drawable.fill_color_t);
#endif
#ifndef HAS_UNIFORM_u_halo_color
    out.halo_color = unpack_mix_color(in.halo_color, drawable.halo_color_t);
#endif
#ifndef HAS_UNIFORM_u_halo_width
    out.halo_width = unpack_mix_float(vec2<f32>(in.halo_width, in.halo_width), drawable.halo_width_t);
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    out.halo_blur = unpack_mix_float(vec2<f32>(in.halo_blur, in.halo_blur), drawable.halo_blur_t);
#endif
#ifndef HAS_UNIFORM_u_opacity
    out.opacity = unpack_mix_float(vec2<f32>(in.opacity, in.opacity), drawable.opacity_t);
#endif

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
    @location(2) gamma_scale: f32,
    @location(3) fontScale: f32,
#ifndef HAS_UNIFORM_u_fill_color
    @location(4) fill_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_halo_color
    @location(5) halo_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(6) opacity: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_width
    @location(7) halo_width: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    @location(8) halo_blur: f32,
#endif
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<SymbolTilePropsUBO>;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(4) var<uniform> props: SymbolEvaluatedPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var glyph_atlas: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];
#ifndef HAS_UNIFORM_u_fill_color
    let fill_color = in.fill_color;
#else
    let fill_color = select(props.icon_fill_color, props.text_fill_color, tileProps.is_text != 0u);
#endif
#ifndef HAS_UNIFORM_u_halo_color
    let halo_color = in.halo_color;
#else
    let halo_color = select(props.icon_halo_color, props.text_halo_color, tileProps.is_text != 0u);
#endif
#ifndef HAS_UNIFORM_u_opacity
    let opacity = in.opacity;
#else
    let opacity = select(props.icon_opacity, props.text_opacity, tileProps.is_text != 0u);
#endif
#ifndef HAS_UNIFORM_u_halo_width
    let halo_width = in.halo_width;
#else
    let halo_width = select(props.icon_halo_width, props.text_halo_width, tileProps.is_text != 0u);
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    let halo_blur = in.halo_blur;
#else
    let halo_blur = select(props.icon_halo_blur, props.text_halo_blur, tileProps.is_text != 0u);
#endif

    let EDGE_GAMMA = 0.105 / paintParams.pixel_ratio;
    let fontGamma = in.fontScale * tileProps.gamma_scale;
    let color = select(fill_color, halo_color, tileProps.is_halo != 0u);
    let gamma = (select(0.0, halo_blur * 1.19 / SDF_PX, tileProps.is_halo != 0u) + EDGE_GAMMA) / fontGamma;
    let gamma_scaled = gamma * in.gamma_scale;

    // For halos, offset the inner edge outward to create the ring
    var inner_edge = (256.0 - 64.0) / 256.0;
    if (tileProps.is_halo != 0u) {
        inner_edge = inner_edge + gamma_scaled;
    }

    let sample = textureSample(glyph_atlas, texture_sampler, in.tex);
    let dist = select(sample.a, sample.r, tileProps.is_text != 0u);
    var alpha = smoothstep(inner_edge - gamma_scaled, inner_edge + gamma_scaled, dist);

    // When drawing halos, make the inside transparent so the fill can show through
    if (tileProps.is_halo != 0u) {
        let halo_edge = (6.0 - halo_width / in.fontScale) / SDF_PX;
        alpha = min(smoothstep(halo_edge - gamma_scaled, halo_edge + gamma_scaled, dist), 1.0 - alpha);
    }

    let coverage = alpha * opacity * in.fade_opacity;
    let outAlpha = color.a * coverage;

    return vec4<f32>(color.rgb * coverage, outAlpha);
}
)";
};

template <>
struct ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "SymbolTextAndIconShader";
    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;
    static constexpr auto prelude = symbolShaderPrelude;

    static constexpr auto vertex = R"()"
                                   R"(
struct VertexInput {
    @location(3) pos_offset: vec4<i32>,
    @location(4) data: vec4<u32>,
    @location(5) projected_pos: vec3<f32>,
    @location(6) fade_opacity: f32,
#ifndef HAS_UNIFORM_u_fill_color
    @location(7) fill_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_halo_color
    @location(8) halo_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(9) opacity: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_width
    @location(10) halo_width: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    @location(11) halo_blur: f32,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
    @location(2) gamma_scale: f32,
    @location(3) fontScale: f32,
    @location(4) is_icon: f32,
#ifndef HAS_UNIFORM_u_fill_color
    @location(5) fill_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_halo_color
    @location(6) halo_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(7) opacity: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_width
    @location(8) halo_width: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    @location(9) halo_blur: f32,
#endif
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<SymbolDrawableUBO>;

const SDF = 1.0;
const ICON = 0.0;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let drawable = drawableVector[globalIndex.value];

    let fade_opacity = unpack_opacity(in.fade_opacity);
    let fade_change = select(-paintParams.symbol_fade_change, paintParams.symbol_fade_change, fade_opacity.y > 0.5);
    let fo = max(0.0, min(1.0, fade_opacity.x + fade_change));

    // Cull vertices with zero opacity
    if (fo == 0.0) {
        out.position = vec4<f32>(c_offscreen_degenerate_triangle_location,
                                 c_offscreen_degenerate_triangle_location,
                                 c_offscreen_degenerate_triangle_location, 1.0);
        return out;
    }

    let a_pos = vec2<f32>(f32(in.pos_offset.x), f32(in.pos_offset.y));
    let a_offset = vec2<f32>(f32(in.pos_offset.z), f32(in.pos_offset.w));

    let a_tex = vec2<f32>(f32(in.data.x), f32(in.data.y));
    let a_size = vec2<f32>(f32(in.data.z), f32(in.data.w));

    let a_size_min = floor(a_size.x * 0.5);
    let is_sdf = a_size.x - 2.0 * a_size_min;

    let segment_angle = -in.projected_pos.z;

    let size_zoom_constant = drawable.is_size_zoom_constant != 0u;
    let size_feature_constant = drawable.is_size_feature_constant != 0u;
    var size: f32;
    if (!size_zoom_constant && !size_feature_constant) {
        size = mix(a_size_min, a_size.y, drawable.size_t) / 128.0;
    } else if (size_zoom_constant && !size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = drawable.size;
    }

    let projectedPoint = drawable.matrix * vec4<f32>(a_pos, 0.0, 1.0);
    let camera_to_anchor_distance = projectedPoint.w;
    let pitch_with_map = drawable.pitch_with_map != 0u;
    let distance_ratio = select(
        paintParams.camera_to_center_distance / camera_to_anchor_distance,
        camera_to_anchor_distance / paintParams.camera_to_center_distance,
        pitch_with_map
    );
    let perspective_ratio = clamp(0.5 + 0.5 * distance_ratio, 0.0, 4.0);

    if (drawable.is_offset == 0u) {
        size *= perspective_ratio;
    }

    let fontScale = size / 24.0;

    var symbol_rotation = 0.0;
    if (drawable.rotate_symbol != 0u) {
        let offsetProjectedPoint = drawable.matrix * vec4<f32>(a_pos + vec2<f32>(1.0, 0.0), 0.0, 1.0);
        let a = projectedPoint.xy / projectedPoint.w;
        let b = offsetProjectedPoint.xy / offsetProjectedPoint.w;
        symbol_rotation = atan2((b.y - a.y) / paintParams.aspect_ratio, b.x - a.x);
    }

    let angle_sin = sin(segment_angle + symbol_rotation);
    let angle_cos = cos(segment_angle + symbol_rotation);
    let rotation_matrix = mat2x2<f32>(angle_cos, -angle_sin, angle_sin, angle_cos);

    let projected_pos = drawable.label_plane_matrix * vec4<f32>(in.projected_pos.xy, 0.0, 1.0);
    let pos_rot = a_offset / 32.0 * fontScale;
    let pos0 = projected_pos.xy / projected_pos.w + rotation_matrix * pos_rot;
    let position = drawable.coord_matrix * vec4<f32>(pos0, 0.0, 1.0);
    let gamma_scale = position.w;
    let is_icon = select(1.0, 0.0, is_sdf == SDF);

    out.position = position;

    let inv_tex = 1.0 / select(drawable.texsize, drawable.texsize_icon, is_icon != 0.0);
    out.tex = a_tex * inv_tex;
    out.gamma_scale = gamma_scale;
    out.fontScale = fontScale;
    out.fade_opacity = fo;
    out.is_icon = is_icon;

#ifndef HAS_UNIFORM_u_fill_color
    out.fill_color = unpack_mix_color(in.fill_color, drawable.fill_color_t);
#endif
#ifndef HAS_UNIFORM_u_halo_color
    out.halo_color = unpack_mix_color(in.halo_color, drawable.halo_color_t);
#endif
#ifndef HAS_UNIFORM_u_opacity
    out.opacity = unpack_mix_float(vec2<f32>(in.opacity, in.opacity), drawable.opacity_t);
#endif
#ifndef HAS_UNIFORM_u_halo_width
    out.halo_width = unpack_mix_float(vec2<f32>(in.halo_width, in.halo_width), drawable.halo_width_t);
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    out.halo_blur = unpack_mix_float(vec2<f32>(in.halo_blur, in.halo_blur), drawable.halo_blur_t);
#endif

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
    @location(2) gamma_scale: f32,
    @location(3) fontScale: f32,
    @location(4) is_icon: f32,
#ifndef HAS_UNIFORM_u_fill_color
    @location(5) fill_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_halo_color
    @location(6) halo_color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_opacity
    @location(7) opacity: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_width
    @location(8) halo_width: f32,
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    @location(9) halo_blur: f32,
#endif
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<SymbolTilePropsUBO>;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(4) var<uniform> props: SymbolEvaluatedPropsUBO;
@group(1) @binding(0) var glyph_sampler: sampler;
@group(1) @binding(1) var glyph_image: texture_2d<f32>;
@group(1) @binding(2) var icon_sampler: sampler;
@group(1) @binding(3) var icon_image: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];
#ifndef HAS_UNIFORM_u_fill_color
    let fill_color = in.fill_color;
#else
    let fill_color = select(props.icon_fill_color, props.text_fill_color, tileProps.is_text != 0u);
#endif
#ifndef HAS_UNIFORM_u_halo_color
    let halo_color = in.halo_color;
#else
    let halo_color = select(props.icon_halo_color, props.text_halo_color, tileProps.is_text != 0u);
#endif
#ifndef HAS_UNIFORM_u_opacity
    let opacity = in.opacity;
#else
    let opacity = select(props.icon_opacity, props.text_opacity, tileProps.is_text != 0u);
#endif
#ifndef HAS_UNIFORM_u_halo_width
    let halo_width = in.halo_width;
#else
    let halo_width = select(props.icon_halo_width, props.text_halo_width, tileProps.is_text != 0u);
#endif
#ifndef HAS_UNIFORM_u_halo_blur
    let halo_blur = in.halo_blur;
#else
    let halo_blur = select(props.icon_halo_blur, props.text_halo_blur, tileProps.is_text != 0u);
#endif

    if (in.is_icon != 0.0) {
        let alpha = opacity * in.fade_opacity;
        let iconSample = textureSampleLevel(icon_image, icon_sampler, in.tex, 0.0);
        return iconSample * alpha;
    }

    let EDGE_GAMMA = 0.105 / paintParams.pixel_ratio;
    let color = select(fill_color, halo_color, tileProps.is_halo != 0u);
    let fontGamma = in.fontScale * tileProps.gamma_scale;
    let gamma = (select(0.0, halo_blur * 1.19 / SDF_PX, tileProps.is_halo != 0u) + EDGE_GAMMA) / fontGamma;
    let gamma_scaled = gamma * in.gamma_scale;

    // For halos, offset the inner edge outward to create the ring
    var inner_edge = (256.0 - 64.0) / 256.0;
    if (tileProps.is_halo != 0u) {
        inner_edge = inner_edge + gamma_scaled;
    }

    let sample = textureSampleLevel(glyph_image, glyph_sampler, in.tex, 0.0);
    let dist = select(sample.a, sample.r, tileProps.is_text != 0u);
    var alpha = smoothstep(inner_edge - gamma_scaled, inner_edge + gamma_scaled, dist);

    // When drawing halos, make the inside transparent so the fill can show through
    if (tileProps.is_halo != 0u) {
        let halo_edge = (6.0 - halo_width / in.fontScale) / SDF_PX;
        alpha = min(smoothstep(halo_edge - gamma_scaled, halo_edge + gamma_scaled, dist), 1.0 - alpha);
    }

    let coverage = alpha * opacity * in.fade_opacity;
    let outAlpha = color.a * coverage;

    return vec4<f32>(color.rgb * coverage, outAlpha);

}
)";
};

} // namespace shaders
} // namespace mbgl
