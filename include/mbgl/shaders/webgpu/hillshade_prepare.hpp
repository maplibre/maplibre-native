#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "HillshadePrepareShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<i32>,
    @location(6) texcoord: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex_coord: vec2<f32>,
};

struct HillshadePrepareUBO {
    matrix: mat4x4<f32>,
    dimension: vec2<f32>,
    zoom: f32,
    maxzoom: f32,
};

@group(0) @binding(0) var<uniform> ubo: HillshadePrepareUBO;

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
};

struct HillshadePrepareUBO {
    matrix: mat4x4<f32>,
    dimension: vec2<f32>,
    zoom: f32,
    maxzoom: f32,
};

@group(0) @binding(0) var<uniform> ubo: HillshadePrepareUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var dem_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let height = textureSample(dem_texture, texture_sampler, in.tex_coord).r;
    
    // Simple encoding of height for hillshade preparation
    return vec4<f32>(height, height, height, 1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl