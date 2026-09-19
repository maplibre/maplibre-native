#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/webgpu/shader_program.hpp>
#include <mln/shaders/location_indicator_ubo.hpp>

namespace mln {
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
    @location(0) local: vec2<f32>,
};

struct LocationIndicatorUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
    sector: vec4<f32>,
};

@group(0) @binding(4) var<uniform> ubo: LocationIndicatorUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = ubo.matrix * vec4<f32>(in.position, 0.0, 1.0);
    out.local = in.position;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct LocationIndicatorUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
    sector: vec4<f32>,
};

@group(0) @binding(4) var<uniform> ubo: LocationIndicatorUBO;

@fragment
fn main(@location(0) local: vec2<f32>) -> @location(0) vec4<f32> {
    var opacity = 1.0;
    if (ubo.sector.y > 0.0) {
        let radius = length(local);
        let angle = atan2(max(abs(local.x), 0.000001), -local.y);
        let feather = length(fwidth(local)) / max(radius, 0.0001);
        let angular = select(
            1.0 - smoothstep(ubo.sector.x - feather, ubo.sector.x + feather, angle),
            1.0, ubo.sector.x >= 3.14159265);
        opacity = angular * (1.0 - smoothstep(0.0, 1.0, radius));
    }
    return ubo.color * opacity;
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
    sector: vec4<f32>,
};

@group(0) @binding(4) var<uniform> ubo: LocationIndicatorUBO;

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
    sector: vec4<f32>,
};

@group(0) @binding(4) var<uniform> ubo: LocationIndicatorUBO;
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
} // namespace mln
