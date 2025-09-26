#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/location_indicator_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LocationIndicatorShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LocationIndicatorShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
};

struct LocationIndicatorUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
};

@group(0) @binding(0) var<uniform> ubo: LocationIndicatorUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = ubo.matrix * vec4<f32>(in.position, 0.0, 1.0);
    return out;
}
)";

    static constexpr auto fragment = R"(
struct LocationIndicatorUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
};

@group(0) @binding(0) var<uniform> ubo: LocationIndicatorUBO;

@fragment
fn main(@builtin(position) position: vec4<f32>) -> @location(0) vec4<f32> {
    return ubo.color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::LocationIndicatorTexturedShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LocationIndicatorTexturedShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<f32>,
    @location(6) uv: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
};

struct LocationIndicatorUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
};

@group(0) @binding(0) var<uniform> ubo: LocationIndicatorUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = ubo.matrix * vec4<f32>(in.position, 0.0, 1.0);
    out.uv = in.uv;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct LocationIndicatorUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
};

@group(0) @binding(0) var<uniform> ubo: LocationIndicatorUBO;
@group(0) @binding(1) var colorTexture: texture_2d<f32>;
@group(0) @binding(2) var colorSampler: sampler;

@fragment
fn main(@location(0) uv: vec2<f32>) -> @location(0) vec4<f32> {
    let textureColor = textureSample(colorTexture, colorSampler, uv);
    return textureColor * ubo.color;
}
)";
};

} // namespace shaders
} // namespace mbgl
