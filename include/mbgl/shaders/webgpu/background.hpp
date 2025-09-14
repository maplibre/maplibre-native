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
    out.position = ubo.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
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
    static const std::array<TextureInfo, 2> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex_coord: vec2<f32>,
};

struct BackgroundPatternUBO {
    matrix: mat4x4<f32>,
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    opacity: f32,
    mix_factor: f32,
};

@group(0) @binding(0) var<uniform> ubo: BackgroundPatternUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = ubo.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    out.tex_coord = vec2<f32>(f32(in.position.x), f32(in.position.y));
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex_coord: vec2<f32>,
};

struct BackgroundPatternUBO {
    matrix: mat4x4<f32>,
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    opacity: f32,
    mix_factor: f32,
};

@group(0) @binding(0) var<uniform> ubo: BackgroundPatternUBO;
@group(0) @binding(1) var texture_sampler: sampler;
@group(0) @binding(2) var texture_from: texture_2d<f32>;
@group(0) @binding(3) var texture_to: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let color_from = textureSample(texture_from, texture_sampler, in.tex_coord);
    let color_to = textureSample(texture_to, texture_sampler, in.tex_coord);
    let color = mix(color_from, color_to, ubo.mix_factor);
    return color * ubo.opacity;
}
)";
};

} // namespace shaders
} // namespace mbgl