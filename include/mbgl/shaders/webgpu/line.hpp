#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
structstruct ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LineShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) pos_normal: vec4<f32>,  // xy = position, zw = normal
    @location(1) data: vec4<f32>,        // xy = extrude, z = linesofar, w = reserved
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) normal: vec2<f32>,
    @location(1) gamma_scale: f32,
    @location(2) v_linesofar: f32,
};

struct LineUBO {
    matrix: mat4x4<f32>,
    ratio: f32,
    half_width: f32,
    pad1: f32,
    pad2: f32,
};

@group(0) @binding(0) var<uniform> line_ubo: LineUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    let pos = in.pos_normal.xy;
    let normal = in.pos_normal.zw;
    let extrude = in.data.xy;
    let linesofar = in.data.z;
    
    // Scale the extrusion vector by the line width
    let dist = length(extrude) * line_ubo.half_width;
    
    // Calculate the position by extruding along the normal
    let offset = normal * dist;
    out.position = line_ubo.matrix * vec4<f32>(pos + offset, 0.0, 1.0);
    
    out.normal = normal;
    out.gamma_scale = 1.0 / (dist * line_ubo.ratio);
    out.v_linesofar = linesofar;
    
    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) normal: vec2<f32>,
    @location(1) gamma_scale: f32,
    @location(2) v_linesofar: f32,
};

struct LinePropsUBO {
    color: vec4<f32>,
    blur: f32,
    opacity: f32,
    gapwidth: f32,
    offset: f32,
    width: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

@group(0) @binding(1) var<uniform> line_props: LinePropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Calculate antialiasing
    let blur = line_props.blur + 1.0 / 200.0;
    let dist = length(in.normal) * line_props.width;
    
    // Apply antialiasing
    let alpha = clamp(min(dist - (line_props.width - blur), line_props.width - dist) / blur, 0.0, 1.0);
    
    return vec4<f32>(line_props.color.rgb, line_props.color.a * line_props.opacity * alpha);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LineGradientShader";
    static const std::array<AttributeInfo, 7> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
};

template <>
struct ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LinePatternShader";
    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
};

template <>
struct ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LineGradientShader";
    static const std::array<AttributeInfo, 7> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

template <>
struct ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LinePatternShader";
    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

template <>
struct ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LineSDFShader";
    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;
    
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

} // namespace shaders
} // namespace mbgl