#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillShader";
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr const char* vertex = R"(
// Helper functions for unpacking colors
fn unpack_float(packedValue: f32) -> vec2<f32> {
    let packedIntValue = i32(packedValue);
    let v0 = packedIntValue / 256;
    return vec2<f32>(f32(v0), f32(packedIntValue - v0 * 256));
}

fn decode_color(encoded: vec2<f32>) -> vec4<f32> {
    let e0 = unpack_float(encoded[0]) / 255.0;
    let e1 = unpack_float(encoded[1]) / 255.0;
    return vec4<f32>(e0.x, e0.y, e1.x, e1.y);
}

fn unpack_mix_color(packedColors: vec4<f32>, t: f32) -> vec4<f32> {
    let color1 = decode_color(vec2<f32>(packedColors[0], packedColors[1]));
    let color2 = decode_color(vec2<f32>(packedColors[2], packedColors[3]));
    return mix(color1, color2, t);
}

fn unpack_mix_float(packedValue: vec2<f32>, t: f32) -> f32 {
    return mix(packedValue[0], packedValue[1], t);
}

struct VertexInput {
    @location(0) position: vec2<i16>,
    @location(1) color: vec4<f32>,
    @location(2) opacity: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
};

struct FillDrawableUBO {
    matrix: mat4x4<f32>,
    color_t: f32,
    opacity_t: f32,
    pad1: f32,
    pad2: f32,
};

// Use the same binding index as Metal: idFillDrawableUBO = 2
@group(0) @binding(2) var<uniform> drawable: FillDrawableUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    // Convert integer position to float
    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));

    // Apply matrix transformation
    out.position = drawable.matrix * vec4<f32>(pos, 0.0, 1.0);

    // Unpack and mix color based on interpolation factor
    out.color = unpack_mix_color(in.color, drawable.color_t);

    // Unpack and mix opacity
    out.opacity = unpack_mix_float(in.opacity, drawable.opacity_t);

    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
};

struct FillEvaluatedPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
};

// Use the same binding index as Metal: idFillEvaluatedPropsUBO = 3 (Metal/WebGPU, not Vulkan)
@group(0) @binding(3) var<uniform> props: FillEvaluatedPropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Use interpolated color from vertex shader
    let final_color = in.color;
    // Combine vertex opacity with uniform opacity
    let final_opacity = in.opacity * props.opacity;

    return vec4<f32>(final_color.rgb, final_color.a * final_opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillOutlineShader";
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
    @location(1) outline_color: vec4<f32>,
    @location(2) opacity: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) outline_color: vec4<f32>,
    @location(1) opacity: f32,
};

struct FillOutlineUBO {
    matrix: mat4x4<f32>,
    ratio: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(0) var<uniform> outline_ubo: FillOutlineUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    out.position = outline_ubo.matrix * vec4<f32>(pos, 0.0, 1.0);
    out.outline_color = in.outline_color;
    out.opacity = in.opacity.x;
    
    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) outline_color: vec4<f32>,
    @location(1) opacity: f32,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    return in.outline_color * in.opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillPatternShader";
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    // Pattern shader implementations would go here
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

template <>
struct ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillOutlinePatternShader";
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillOutlineTriangulatedShader";
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

} // namespace shaders
} // namespace mbgl