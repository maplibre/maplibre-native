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
struct VertexInput {
    @location(0) position: vec2<i32>,
    @location(1) color: vec4<f32>,
    @location(2) opacity: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
};

struct FillUBO {
    matrix: mat4x4<f32>,
    color_t: f32,
    opacity_t: f32,
    pad1: f32,
    pad2: f32,
};

@group(0) @binding(0) var<uniform> fill_ubo: FillUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    // Convert integer position to float
    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    
    // Apply matrix transformation
    out.position = fill_ubo.matrix * vec4<f32>(pos, 0.0, 1.0);
    
    // Mix color based on interpolation factor
    out.color = mix(in.color, in.color, fill_ubo.color_t);
    
    // Mix opacity
    out.opacity = mix(in.opacity.x, in.opacity.y, fill_ubo.opacity_t);
    
    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) color: vec4<f32>,
    @location(1) opacity: f32,
};

struct FillPropsUBO {
    color: vec4<f32>,
    outline_color: vec4<f32>,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
};

@group(0) @binding(1) var<uniform> props: FillPropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    // Use either uniform color or interpolated color
    let final_color = in.color;
    let final_opacity = in.opacity * props.opacity;
    
    return final_color * final_opacity;
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
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr const char* vertex = "";
    static constexpr const char* fragment = "";
};

} // namespace shaders
} // namespace mbgl