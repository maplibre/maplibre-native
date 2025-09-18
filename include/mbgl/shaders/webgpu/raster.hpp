#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/raster_layer_ubo.hpp>

namespace mbgl {
namespace shaders {


template <>
struct ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "RasterShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;
    
    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
    @location(1) texcoord: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex_coord: vec2<f32>,
};

struct RasterUBO {
    matrix: mat4x4<f32>,
    opacity: f32,
    fade_t: f32,
    brightness_low: f32,
    brightness_high: f32,
    saturation_factor: f32,
    contrast_factor: f32,
    spin_weights: vec3<f32>,
};

@group(0) @binding(0) var<uniform> ubo: RasterUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = ubo.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    out.tex_coord = vec2<f32>(f32(in.texcoord.x), f32(in.texcoord.y)) / 8192.0;
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex_coord: vec2<f32>,
};

struct RasterUBO {
    matrix: mat4x4<f32>,
    opacity: f32,
    fade_t: f32,
    brightness_low: f32,
    brightness_high: f32,
    saturation_factor: f32,
    contrast_factor: f32,
    spin_weights: vec3<f32>,
};

@group(0) @binding(0) var<uniform> ubo: RasterUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var image0: texture_2d<f32>;
@group(1) @binding(2) var image1: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let color0 = textureSample(image0, texture_sampler, in.tex_coord);
    let color1 = textureSample(image1, texture_sampler, in.tex_coord);
    
    var color = mix(color0, color1, ubo.fade_t);
    color = vec4<f32>(color.rgb * color.a, color.a);
    
    // Apply brightness
    let brightness = mix(ubo.brightness_low, ubo.brightness_high, color.r);
    color = vec4<f32>(color.rgb * brightness, color.a);
    
    // Apply saturation
    let gray = dot(color.rgb, vec3<f32>(0.299, 0.587, 0.114));
    color = vec4<f32>(mix(vec3<f32>(gray, gray, gray), color.rgb, ubo.saturation_factor), color.a);
    
    // Apply contrast
    color = vec4<f32>((color.rgb - 0.5) * ubo.contrast_factor + 0.5, color.a);
    
    // Apply opacity
    return color * ubo.opacity;
}
)";
};

} // namespace shaders
} // namespace mbgl