#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/custom_geometry_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CustomGeometryShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "CustomGeometryShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec3<f32>,
    @location(4) texcoord: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
};

struct CustomDrawableUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
};

@group(0) @binding(0) var<uniform> drawable: CustomDrawableUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.uv = in.texcoord;
    out.position = drawable.matrix * vec4<f32>(in.position, 1.0);
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) uv: vec2<f32>,
};

struct CustomDrawableUBO {
    matrix: mat4x4<f32>,
    color: vec4<f32>,
};

@group(0) @binding(0) var<uniform> drawable: CustomDrawableUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var texture_image: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    return textureSample(texture_image, texture_sampler, in.uv) * drawable.color;
}
)";
};

} // namespace shaders
} // namespace mbgl
