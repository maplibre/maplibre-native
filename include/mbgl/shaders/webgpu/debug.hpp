#pragma once

#include <mbgl/shaders/debug_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "DebugShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
};

struct DebugUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
    overlay_scale: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(0) var<uniform> debug: DebugUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let scaled_pos = vec2<f32>(f32(in.position.x), f32(in.position.y)) * debug.overlay_scale;
    out.position = debug.matrix * vec4<f32>(scaled_pos, 0.0, 1.0);

    // This vertex shader expects a EXTENT x EXTENT quad,
    // The UV co-ordinates for the overlay texture can be calculated using that knowledge
    out.uv = vec2<f32>(f32(in.position.x), f32(in.position.y)) / 8192.0;

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) uv: vec2<f32>,
};

struct DebugUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
    overlay_scale: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(0) var<uniform> debug: DebugUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var overlay_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let overlay_color = textureSample(overlay_texture, texture_sampler, in.uv);
    return mix(debug.color, overlay_color, overlay_color.a);
}
)";
};

} // namespace shaders
} // namespace mbgl
