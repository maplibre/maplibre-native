#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "BackgroundShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
};

struct BackgroundUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
    opacity: f32,
};

@group(0) @binding(0) var<uniform> ubo: BackgroundUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let transformed = ubo.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    // Convert to NDC coordinates and flip Y for WebGPU
    // WebGPU uses Z range [0, 1] instead of [-1, 1]
    // Background should be at the far plane (depth = 1.0)
    let ndc_z = 0.999;
    out.position = vec4<f32>(
        transformed.x / transformed.w,
        -transformed.y / transformed.w,
        ndc_z,
        1.0
    );
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct BackgroundUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
    opacity: f32,
};

@group(0) @binding(0) var<uniform> ubo: BackgroundUBO;

@fragment
fn main() -> @location(0) vec4<f32> {
    return ubo.color * ubo.opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "BackgroundPatternShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) pos_a: vec2<f32>,
    @location(1) pos_b: vec2<f32>,
};

struct BackgroundPatternDrawableUBO {
    matrix: mat4x4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    tile_units_to_pixels: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

struct BackgroundPatternPropsUBO {
    pattern_tl_a: vec2<f32>,
    pattern_br_a: vec2<f32>,
    pattern_tl_b: vec2<f32>,
    pattern_br_b: vec2<f32>,
    pattern_size_a: vec2<f32>,
    pattern_size_b: vec2<f32>,
    scale_a: f32,
    scale_b: f32,
    mix_val: f32,
    opacity: f32,
    pad1: f32,
    pad2: f32,
};

@group(0) @binding(2) var<uniform> drawable: BackgroundPatternDrawableUBO;
@group(0) @binding(5) var<uniform> props: BackgroundPatternPropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));

    // Use get_pattern_pos helper function for pattern positioning
    out.pos_a = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        props.scale_a * props.pattern_size_a,
        drawable.tile_units_to_pixels,
        pos
    );

    out.pos_b = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        props.scale_b * props.pattern_size_b,
        drawable.tile_units_to_pixels,
        pos
    );

    let clip = drawable.matrix * vec4<f32>(pos, 0.0, 1.0);
    let invW = 1.0 / clip.w;
    let ndcZ = (clip.z * invW) * 0.5 + 0.5;
    out.position = vec4<f32>(clip.x * invW,
                             -clip.y * invW,
                             ndcZ,
                             1.0);
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) pos_a: vec2<f32>,
    @location(1) pos_b: vec2<f32>,
};

struct BackgroundPatternPropsUBO {
    pattern_tl_a: vec2<f32>,
    pattern_br_a: vec2<f32>,
    pattern_tl_b: vec2<f32>,
    pattern_br_b: vec2<f32>,
    pattern_size_a: vec2<f32>,
    pattern_size_b: vec2<f32>,
    scale_a: f32,
    scale_b: f32,
    mix_val: f32,
    opacity: f32,
    pad1: f32,
    pad2: f32,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    // other fields...
};

@group(0) @binding(4) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(5) var<uniform> props: BackgroundPatternPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let texsize = paintParams.pattern_atlas_texsize;

    // Sample pattern A
    let imagecoord_a = gl_mod(in.pos_a, vec2<f32>(1.0));
    let pos_a = mix(props.pattern_tl_a / texsize, props.pattern_br_a / texsize, imagecoord_a);
    let color_a = textureSample(pattern_texture, texture_sampler, pos_a);

    // Sample pattern B
    let imagecoord_b = gl_mod(in.pos_b, vec2<f32>(1.0));
    let pos_b = mix(props.pattern_tl_b / texsize, props.pattern_br_b / texsize, imagecoord_b);
    let color_b = textureSample(pattern_texture, texture_sampler, pos_b);

    // Mix patterns and apply opacity
    return mix(color_a, color_b, props.mix_val) * props.opacity;
}
)";
};

} // namespace shaders
} // namespace mbgl
