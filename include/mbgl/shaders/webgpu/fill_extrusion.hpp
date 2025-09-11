#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillExtrusionShader";
    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
    @location(1) normal_ed: vec4<i32>,
    @location(2) color: vec4<f32>,
    @location(3) base: f32,
    @location(4) height: f32,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
};

struct FillExtrusionDrawableUBO {
    matrix: mat4x4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    height_factor: f32,
    tile_ratio: f32,
    base_t: f32,
    height_t: f32,
    color_t: f32,
    pattern_from_t: f32,
    pattern_to_t: f32,
    pad1: f32,
};

struct FillExtrusionPropsUBO {
    color: vec4<f32>,
    light_color_pad: vec4<f32>,
    light_position_base: vec4<f32>,
    height: f32,
    light_intensity: f32,
    vertical_gradient: f32,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
    pad2: f32,
};

@group(0) @binding(0) var<storage, read> drawableVector: array<FillExtrusionDrawableUBO>;
@group(0) @binding(1) var<uniform> props: FillExtrusionPropsUBO;
@group(0) @binding(2) var<uniform> uboIndex: u32;

fn unpack_mix_float(value: f32, t: f32) -> f32 {
    return value;
}

fn unpack_mix_color(packed: vec4<f32>, t: f32) -> vec4<f32> {
    return packed;
}

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[uboIndex];
    
    let base = max(unpack_mix_float(in.base, drawable.base_t), 0.0);
    let height = max(unpack_mix_float(in.height, drawable.height_t), 0.0);
    
    let normal = vec3<f32>(f32(in.normal_ed.x), f32(in.normal_ed.y), f32(in.normal_ed.z));
    let t = mix(base, height, f32(in.normal_ed.w));
    
    let pos = vec3<f32>(f32(in.position.x), f32(in.position.y), t);
    
    out.position = drawable.matrix * vec4<f32>(pos, 1.0);
    
    // Calculate lighting
    let light_position = props.light_position_base.xyz;
    let light_color = props.light_color_pad.xyz;
    let light_intensity = props.light_intensity;
    
    // Simple directional lighting
    let light_dir = normalize(light_position);
    let n_dot_l = max(dot(normal, light_dir), 0.0);
    
    let base_color = unpack_mix_color(in.color, drawable.color_t);
    let lit_color = base_color * vec4<f32>(light_color * light_intensity * n_dot_l, 1.0);
    
    // Apply vertical gradient
    let vertical_gradient = props.vertical_gradient;
    let gradient_factor = mix(1.0, 0.5, vertical_gradient * (height - base) / (props.height));
    
    out.color = vec4<f32>(lit_color.rgb * gradient_factor, base_color.a * props.opacity);
    
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) color: vec4<f32>,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    return in.color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillExtrusionPatternShader";
    static const std::array<AttributeInfo, 6> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
    @location(1) normal_ed: vec4<i32>,
    @location(2) color: vec4<f32>,
    @location(3) base: f32,
    @location(4) height: f32,
    @location(5) pattern: vec4<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) pattern_pos: vec2<f32>,
};

struct FillExtrusionDrawableUBO {
    matrix: mat4x4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    height_factor: f32,
    tile_ratio: f32,
    base_t: f32,
    height_t: f32,
    color_t: f32,
    pattern_from_t: f32,
    pattern_to_t: f32,
    pad1: f32,
};

@group(0) @binding(0) var<storage, read> drawableVector: array<FillExtrusionDrawableUBO>;
@group(0) @binding(1) var<uniform> uboIndex: u32;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[uboIndex];
    
    let base = max(in.base, 0.0);
    let height = max(in.height, 0.0);
    
    let normal = vec3<f32>(f32(in.normal_ed.x), f32(in.normal_ed.y), f32(in.normal_ed.z));
    let t = mix(base, height, f32(in.normal_ed.w));
    
    let pos = vec3<f32>(f32(in.position.x), f32(in.position.y), t);
    
    out.position = drawable.matrix * vec4<f32>(pos, 1.0);
    out.color = in.color;
    out.pattern_pos = in.pattern.xy;
    
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) color: vec4<f32>,
    @location(1) pattern_pos: vec2<f32>,
};

@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let pattern_color = textureSample(pattern_texture, texture_sampler, in.pattern_pos);
    return in.color * pattern_color;
}
)";
};

} // namespace shaders
} // namespace mbgl