#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace shaders {


template <>
struct ShaderSource<BuiltIn::SymbolIconShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "SymbolIconShader";
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) pos_offset: vec4<i32>,
    @location(1) data: vec4<i32>,
    @location(2) pixeloffset: vec4<i32>,
    @location(3) projected_pos: vec3<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
};

struct SymbolDrawableUBO {
    matrix: mat4x4<f32>,
    label_plane_matrix: mat4x4<f32>,
    tex_size: vec2<f32>,
    fade_change: f32,
    is_text: f32,
    camera_to_center_distance: f32,
    pitch: f32,
    pitch_with_map: f32,
    pad: f32,
};

@group(0) @binding(0) var<uniform> drawable: SymbolDrawableUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    let a_pos = vec2<f32>(f32(in.pos_offset.x), f32(in.pos_offset.y));
    let a_offset = vec2<f32>(f32(in.pos_offset.z), f32(in.pos_offset.w));
    
    let a_tex = vec2<f32>(f32(in.data.x), f32(in.data.y));
    let a_size = vec2<f32>(f32(in.data.z), f32(in.data.w));
    
    let projected_pos = vec4<f32>(in.projected_pos, 1.0);
    
    out.position = drawable.matrix * vec4<f32>(a_pos + a_offset, 0.0, 1.0);
    out.tex = a_tex / drawable.tex_size;
    out.fade_opacity = 1.0;
    
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
};

@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var image: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let color = textureSample(image, texture_sampler, in.tex);
    return color * in.fade_opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::SymbolSDFShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "SymbolSDFShader";
    static const std::array<AttributeInfo, 7> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) pos_offset: vec4<i32>,
    @location(1) data: vec4<i32>,
    @location(2) pixeloffset: vec4<i32>,
    @location(3) projected_pos: vec3<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
    @location(2) gamma_scale: f32,
};

struct SymbolDrawableUBO {
    matrix: mat4x4<f32>,
    label_plane_matrix: mat4x4<f32>,
    tex_size: vec2<f32>,
    fade_change: f32,
    is_text: f32,
    camera_to_center_distance: f32,
    pitch: f32,
    pitch_with_map: f32,
    is_halo: f32,
};

struct SymbolTilePropsUBO {
    is_size_zoom_constant: f32,
    is_size_feature_constant: f32,
    size_t: f32,
    size: f32,
    layout_size: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(0) var<uniform> drawable: SymbolDrawableUBO;
@group(0) @binding(1) var<uniform> tile_props: SymbolTilePropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    let a_pos = vec2<f32>(f32(in.pos_offset.x), f32(in.pos_offset.y));
    let a_offset = vec2<f32>(f32(in.pos_offset.z), f32(in.pos_offset.w));
    
    let a_tex = vec2<f32>(f32(in.data.x), f32(in.data.y));
    let a_size = vec2<f32>(f32(in.data.z), f32(in.data.w));
    
    out.position = drawable.matrix * vec4<f32>(a_pos + a_offset * tile_props.size, 0.0, 1.0);
    out.tex = a_tex / drawable.tex_size;
    out.fade_opacity = 1.0;
    out.gamma_scale = 1.0;
    
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
    @location(2) gamma_scale: f32,
};

struct SymbolEvaluatedPropsUBO {
    text_fill_color: vec4<f32>,
    text_halo_color: vec4<f32>,
    text_opacity: f32,
    text_halo_width: f32,
    text_halo_blur: f32,
    pad: f32,
};

@group(0) @binding(2) var<uniform> props: SymbolEvaluatedPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var glyph_atlas: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let dist = textureSample(glyph_atlas, texture_sampler, in.tex).a;
    
    let EDGE_GAMMA = 0.105 / 8.0;
    let gamma = EDGE_GAMMA * in.gamma_scale;
    let alpha = smoothstep(0.5 - gamma, 0.5 + gamma, dist);
    
    var color = props.text_fill_color;
    
    // Apply halo if needed
    if (props.text_halo_width > 0.0) {
        let halo_alpha = smoothstep(0.5 - gamma - props.text_halo_blur, 0.5 + gamma + props.text_halo_blur, dist);
        color = mix(props.text_halo_color, color, alpha);
        alpha = halo_alpha;
    }
    
    return vec4<f32>(color.rgb, color.a * alpha * in.fade_opacity * props.text_opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::SymbolTextAndIconShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "SymbolTextAndIconShader";
    static const std::array<AttributeInfo, 7> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;
    
    // Use the same vertex shader as SymbolSDFShader
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) pos_offset: vec4<i32>,
    @location(1) data: vec4<i32>,
    @location(2) pixeloffset: vec4<i32>,
    @location(3) projected_pos: vec3<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
    @location(2) gamma_scale: f32,
};

struct SymbolDrawableUBO {
    matrix: mat4x4<f32>,
    label_plane_matrix: mat4x4<f32>,
    tex_size: vec2<f32>,
    fade_change: f32,
    is_text: f32,
    camera_to_center_distance: f32,
    pitch: f32,
    pitch_with_map: f32,
    is_halo: f32,
};

struct SymbolTilePropsUBO {
    is_size_zoom_constant: f32,
    is_size_feature_constant: f32,
    size_t: f32,
    size: f32,
    layout_size: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(0) var<uniform> drawable: SymbolDrawableUBO;
@group(0) @binding(1) var<uniform> tile_props: SymbolTilePropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    let a_pos = vec2<f32>(f32(in.pos_offset.x), f32(in.pos_offset.y));
    let a_offset = vec2<f32>(f32(in.pos_offset.z), f32(in.pos_offset.w));
    
    let a_tex = vec2<f32>(f32(in.data.x), f32(in.data.y));
    let a_size = vec2<f32>(f32(in.data.z), f32(in.data.w));
    
    out.position = drawable.matrix * vec4<f32>(a_pos + a_offset * tile_props.size, 0.0, 1.0);
    out.tex = a_tex / drawable.tex_size;
    out.fade_opacity = 1.0;
    out.gamma_scale = 1.0;
    
    return out;
}
)";
    
    // Use the same fragment shader as SymbolSDFShader
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex: vec2<f32>,
    @location(1) fade_opacity: f32,
    @location(2) gamma_scale: f32,
};

struct SymbolEvaluatedPropsUBO {
    text_fill_color: vec4<f32>,
    text_halo_color: vec4<f32>,
    text_opacity: f32,
    text_halo_width: f32,
    text_halo_blur: f32,
    pad: f32,
};

@group(0) @binding(2) var<uniform> props: SymbolEvaluatedPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var glyph_atlas: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let dist = textureSample(glyph_atlas, texture_sampler, in.tex).a;
    
    let EDGE_GAMMA = 0.105 / 8.0;
    let gamma = EDGE_GAMMA * in.gamma_scale;
    let alpha = smoothstep(0.5 - gamma, 0.5 + gamma, dist);
    
    var color = props.text_fill_color;
    
    // Apply halo if needed
    if (props.text_halo_width > 0.0) {
        let halo_alpha = smoothstep(0.5 - gamma - props.text_halo_blur, 0.5 + gamma + props.text_halo_blur, dist);
        color = mix(props.text_halo_color, color, alpha);
        alpha = halo_alpha;
    }
    
    return vec4<f32>(color.rgb, color.a * alpha * in.fade_opacity * props.text_opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl