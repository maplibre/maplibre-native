#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/heatmap_texture_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "HeatmapTextureShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex_coord: vec2<f32>,
};

struct HeatmapTextureUBO {
    matrix: mat4x4<f32>,
    opacity: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    // remaining fields unused
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(4) var<uniform> ubo: HeatmapTextureUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let quad_pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let scaled = quad_pos * paintParams.world_size;

    out.position = ubo.matrix * vec4<f32>(scaled, 0.0, 1.0);
    out.tex_coord = quad_pos;

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex_coord: vec2<f32>,
};

struct HeatmapTextureUBO {
    matrix: mat4x4<f32>,
    opacity: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(4) var<uniform> ubo: HeatmapTextureUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var heatmap_texture: texture_2d<f32>;
@group(1) @binding(2) var color_ramp_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let t = textureSample(heatmap_texture, texture_sampler, in.tex_coord).r;
    let color = textureSample(color_ramp_texture, texture_sampler, vec2<f32>(t, 0.5));
    return color * ubo.opacity;
}
)";
};

} // namespace shaders
} // namespace mbgl
