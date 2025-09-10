#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "BackgroundShader";
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

struct BackgroundUBO {
    matrix: mat4x4<f32>,
};

@group(0) @binding(0) var<uniform> ubo: BackgroundUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = ubo.matrix * vec4<f32>(in.position, 0.0, 1.0);
    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct BackgroundPropsUBO {
    color: vec4<f32>,
    opacity: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(1) var<uniform> props: BackgroundPropsUBO;

@fragment
fn main() -> @location(0) vec4<f32> {
    return vec4<f32>(props.color.rgb, props.color.a * props.opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "BackgroundPatternShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;
    
    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) position: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) pattern_coord: vec2<f32>,
};

struct BackgroundPatternUBO {
    matrix: mat4x4<f32>,
    pattern_matrix: mat3x3<f32>,
    pattern_size: vec2<f32>,
    pixel_ratio: f32,
    pad: f32,
};

@group(0) @binding(0) var<uniform> ubo: BackgroundPatternUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = ubo.matrix * vec4<f32>(in.position, 0.0, 1.0);
    
    // Calculate pattern coordinates
    let pattern_pos = ubo.pattern_matrix * vec3<f32>(in.position, 1.0);
    out.pattern_coord = pattern_pos.xy / ubo.pattern_size;
    
    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) pattern_coord: vec2<f32>,
};

struct BackgroundPatternPropsUBO {
    opacity: f32,
    fade: f32,
    pad1: f32,
    pad2: f32,
};

@group(0) @binding(1) var<uniform> props: BackgroundPatternPropsUBO;
@group(0) @binding(2) var pattern_texture_from: texture_2d<f32>;
@group(0) @binding(3) var pattern_sampler: sampler;
@group(0) @binding(4) var pattern_texture_to: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let color_from = textureSample(pattern_texture_from, pattern_sampler, in.pattern_coord);
    let color_to = textureSample(pattern_texture_to, pattern_sampler, in.pattern_coord);
    
    var color = mix(color_from, color_to, props.fade);
    color.a = color.a * props.opacity;
    
    return color;
}
)";
};

} // namespace shaders
} // namespace mbgl