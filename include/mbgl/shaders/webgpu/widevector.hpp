#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/widevector_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::WideVectorShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "WideVectorShader";
    static const std::array<AttributeInfo, 3> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    // Simplified wide vector shader for WebGPU
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec3<f32>,
    @location(6) normal: vec2<f32>,
    @location(7) texCoord: vec2<f32>,
};

struct InstanceInput {
    @location(8) instanceCenter: vec3<f32>,
    @location(9) instanceColor: vec4<f32>,
    @location(10) instancePrev: i32,
    @location(11) instanceNext: i32,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) texCoord: vec2<f32>,
};

struct WideVectorUBO {
    mvpMatrix: mat4x4<f32>,
    mvpMatrixDiff: mat4x4<f32>,
    mvMatrix: mat4x4<f32>,
    mvMatrixDiff: mat4x4<f32>,
    pMatrix: mat4x4<f32>,
    pMatrixDiff: mat4x4<f32>,
    frameSize: vec2<f32>,
    color: vec4<f32>,
    width: f32,
    offset: f32,
    edge: f32,
    texRepeat: f32,
    texOffset: vec2<f32>,
};

@group(0) @binding(0) var<uniform> ubo: WideVectorUBO;

@vertex
fn main(in: VertexInput, instance: InstanceInput) -> VertexOutput {
    var out: VertexOutput;

    // Calculate position with offset based on normal
    let worldPos = instance.instanceCenter + vec3<f32>(in.normal * ubo.width, 0.0);
    out.position = ubo.mvpMatrix * vec4<f32>(worldPos, 1.0);

    // Pass through color and texture coordinates
    out.color = instance.instanceColor * ubo.color;
    out.texCoord = in.texCoord * ubo.texRepeat + ubo.texOffset;

    return out;
}
)";

    static constexpr auto fragment = R"(
@fragment
fn main(@location(0) color: vec4<f32>, @location(1) texCoord: vec2<f32>) -> @location(0) vec4<f32> {
    // Simple color output with alpha blending support
    return color;
}
)";
};

} // namespace shaders
} // namespace mbgl
