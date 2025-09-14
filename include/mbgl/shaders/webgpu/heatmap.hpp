#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>

namespace mbgl {
namespace shaders {


template <>
struct ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "HeatmapShader";
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
    @location(1) weight: vec2<f32>,
    @location(2) radius: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) weight: f32,
    @location(1) extrude: vec2<f32>,
};

struct HeatmapDrawableUBO {
    matrix: mat4x4<f32>,
    extrude_scale: f32,
    weight_t: f32,
    radius_t: f32,
    pad1: f32,
};

struct HeatmapEvaluatedPropsUBO {
    weight: f32,
    radius: f32,
    intensity: f32,
    padding: f32,
};

@group(0) @binding(0) var<uniform> drawable: HeatmapDrawableUBO;
@group(0) @binding(1) var<uniform> props: HeatmapEvaluatedPropsUBO;

// Effective "0" in the kernel density texture
const ZERO: f32 = 1.0 / 255.0 / 16.0;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    // Get weight and radius using unpack_mix_float helper
    let weight = unpack_mix_float(in.weight, drawable.weight_t);
    let radius = unpack_mix_float(in.radius, drawable.radius_t);
    
    // Decode extrusion vector
    let pos_f = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let extrude_bits = floor(pos_f / 2.0);
    let extrude = pos_f - 2.0 * extrude_bits;
    
    // Scale kernel for this zoom level
    let scaled_extrude = extrude * radius * drawable.extrude_scale;
    
    // Calculate position
    let pos = floor(pos_f * 0.5);
    out.position = drawable.matrix * vec4<f32>(pos + scaled_extrude, 0.0, 1.0);
    
    out.weight = weight;
    out.extrude = extrude;
    
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) weight: f32,
    @location(1) extrude: vec2<f32>,
};

// Gaussian kernel coefficient: 1 / sqrt(2 * PI)
const GAUSS_COEF: f32 = 0.3989422804014327;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Calculate distance from center
    let d = length(in.extrude);
    
    // Gaussian kernel
    let val = in.weight * exp(-0.5 * d * d);
    
    return vec4<f32>(val, val, val, 1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl