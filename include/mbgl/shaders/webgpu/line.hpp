#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::WebGPU> {
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

@group(0) @binding(2) var<uniform> line_ubo: LineUBO;

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

@group(0) @binding(4) var<uniform> line_props: LinePropsUBO;

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
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) pos_normal: vec4<f32>,
    @location(1) data: vec4<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_lineprogress: f32,
};

struct LineGradientUBO {
    matrix: mat4x4<f32>,
};

@group(0) @binding(2) var<uniform> ubo: LineGradientUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let pos = in.pos_normal.xy;
    let normal = in.pos_normal.zw;
    let linesofar = in.data.z;

    out.position = ubo.matrix * vec4<f32>(pos + normal * 2.0, 0.0, 1.0);
    out.v_lineprogress = linesofar;
    return out;
}
)";

    static constexpr const char* fragment = R"(
@group(0) @binding(3) var gradient_texture: texture_2d<f32>;
@group(0) @binding(4) var gradient_sampler: sampler;

@fragment
fn main(@location(0) v_lineprogress: f32) -> @location(0) vec4<f32> {
    let color = textureSample(gradient_texture, gradient_sampler, vec2<f32>(v_lineprogress, 0.5));
    return color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LinePatternShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) pos_normal: vec4<f32>,
    @location(1) data: vec4<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_normal: vec2<f32>,
    @location(1) v_width: f32,
};

struct LinePatternUBO {
    matrix: mat4x4<f32>,
    pattern_size: vec2<f32>,
    tile_units_to_pixels: f32,
    pad: f32,
};

@group(0) @binding(0) var<uniform> ubo: LinePatternUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let pos = in.pos_normal.xy;
    let normal = in.pos_normal.zw;

    out.position = ubo.matrix * vec4<f32>(pos + normal * 2.0, 0.0, 1.0);
    out.v_normal = normal;
    out.v_width = 1.0;
    return out;
}
)";

    static constexpr const char* fragment = R"(
@group(0) @binding(1) var pattern_texture: texture_2d<f32>;
@group(0) @binding(2) var pattern_sampler: sampler;

@fragment
fn main(@location(0) v_normal: vec2<f32>, @location(1) v_width: f32) -> @location(0) vec4<f32> {
    let pattern_pos = v_normal * 0.5 + 0.5;
    let color = textureSample(pattern_texture, pattern_sampler, pattern_pos);
    return color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "LineSDFShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) pos_normal: vec4<f32>,
    @location(1) data: vec4<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) v_normal: vec2<f32>,
    @location(1) v_tex: vec2<f32>,
};

struct LineSDFUBO {
    matrix: mat4x4<f32>,
    patternscale_a: vec2<f32>,
    patternscale_b: vec2<f32>,
    tex_y_a: f32,
    tex_y_b: f32,
    sdfgamma: f32,
    pad: f32,
};

@group(0) @binding(2) var<uniform> ubo: LineSDFUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let pos = in.pos_normal.xy;
    let normal = in.pos_normal.zw;
    let linesofar = in.data.z;

    out.position = ubo.matrix * vec4<f32>(pos + normal * 2.0, 0.0, 1.0);
    out.v_normal = normal;
    out.v_tex = vec2<f32>(linesofar / ubo.patternscale_a.x, normal.y * 0.5 + 0.5);
    return out;
}
)";

    static constexpr const char* fragment = R"(
@group(0) @binding(3) var sdf_texture: texture_2d<f32>;
@group(0) @binding(4) var sdf_sampler: sampler;

@fragment
fn main(@location(0) v_normal: vec2<f32>, @location(1) v_tex: vec2<f32>) -> @location(0) vec4<f32> {
    let dist = textureSample(sdf_texture, sdf_sampler, v_tex).a;
    let alpha = smoothstep(0.5 - 0.03, 0.5 + 0.03, dist);
    return vec4<f32>(1.0, 1.0, 1.0, alpha);
}
)";
};

} // namespace shaders
} // namespace mbgl