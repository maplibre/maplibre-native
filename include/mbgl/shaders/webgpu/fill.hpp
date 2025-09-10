#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) position: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
};

struct FillUBO {
    matrix: mat4x4<f32>,
};

@group(0) @binding(0) var<uniform> fill_ubo: FillUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = fill_ubo.matrix * vec4<f32>(in.position, 0.0, 1.0);
    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FillPropsUBO {
    color: vec4<f32>,
    opacity: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(1) var<uniform> fill_props: FillPropsUBO;

@fragment
fn main() -> @location(0) vec4<f32> {
    return vec4<f32>(fill_props.color.rgb, fill_props.color.a * fill_props.opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillOutlineShader";
    
    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) position: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) pos: vec2<f32>,
};

struct FillOutlineUBO {
    matrix: mat4x4<f32>,
    world: vec2<f32>,
    pad1: f32,
    pad2: f32,
};

@group(0) @binding(0) var<uniform> fill_outline_ubo: FillOutlineUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = fill_outline_ubo.matrix * vec4<f32>(in.position, 0.0, 1.0);
    out.pos = (out.position.xy / out.position.w + 1.0) / 2.0 * fill_outline_ubo.world;
    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) pos: vec2<f32>,
};

struct FillOutlinePropsUBO {
    outline_color: vec4<f32>,
    opacity: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(1) var<uniform> fill_outline_props: FillOutlinePropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let dist = length(fwidth(in.pos));
    let alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    return vec4<f32>(fill_outline_props.outline_color.rgb, fill_outline_props.outline_color.a * fill_outline_props.opacity * alpha);
}
)";
};

} // namespace shaders
} // namespace mbgl