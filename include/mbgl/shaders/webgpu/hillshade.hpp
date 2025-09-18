#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp>

namespace mbgl {
namespace shaders {


template <>
struct ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "HillshadeShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<i32>,
    @location(6) texcoord: vec2<i32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex_coord: vec2<f32>,
}

struct HillshadeUBO {
    matrix: mat4x4<f32>,
    highlight: vec4<f32>,
    shadow: vec4<f32>,
    accent: vec4<f32>,
    light: vec2<f32>,
    latrange: vec2<f32>,
}

@group(0) @binding(0) var<uniform> ubo: HillshadeUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = ubo.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    out.tex_coord = vec2<f32>(f32(in.texcoord.x), f32(in.texcoord.y)) / 8192.0;
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex_coord: vec2<f32>,
}

struct HillshadeUBO {
    matrix: mat4x4<f32>,
    highlight: vec4<f32>,
    shadow: vec4<f32>,
    accent: vec4<f32>,
    light: vec2<f32>,
    latrange: vec2<f32>,
}

@group(0) @binding(0) var<uniform> ubo: HillshadeUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var hillshade_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let color = textureSample(hillshade_texture, texture_sampler, in.tex_coord);
    
    // Simple hillshade calculation
    let shade = color.r;
    let highlight_color = ubo.highlight * shade;
    let shadow_color = ubo.shadow * (1.0 - shade);
    
    return highlight_color + shadow_color + ubo.accent * color.g;
}
)";
};

} // namespace shaders
} // namespace mbgl