#pragma once

#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "CircleShader";
    
    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = R"(
struct CircleDrawableUBO {
    matrix: mat4x4<f32>,
    extrude_scale: vec2<f32>,
    color_t: f32,
    radius_t: f32,
    blur_t: f32,
    opacity_t: f32,
    stroke_color_t: f32,
    stroke_width_t: f32,
    stroke_opacity_t: f32,
    pad1: f32,
    pad2: f32,
    pad3: f32,
};

struct CircleEvaluatedPropsUBO {
    color: vec4<f32>,
    stroke_color: vec4<f32>,
    radius: f32,
    blur: f32,
    opacity: f32,
    stroke_width: f32,
    stroke_opacity: f32,
    scale_with_map: u32,
    pitch_with_map: u32,
    pad1: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<CircleDrawableUBO>;
@group(0) @binding(4) var<uniform> props: CircleEvaluatedPropsUBO;
)";

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
    @location(1) color: vec4<f32>,
    @location(2) radius: vec2<f32>,
    @location(3) blur: vec2<f32>,
    @location(4) opacity: vec2<f32>,
    @location(5) stroke_color: vec4<f32>,
    @location(6) stroke_width: vec2<f32>,
    @location(7) stroke_opacity: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) extrude: vec2<f32>,
    @location(1) antialiasblur: f32,
    @location(2) color: vec4<f32>,
    @location(3) radius: f32,
    @location(4) blur: f32,
    @location(5) opacity: f32,
    @location(6) stroke_color: vec4<f32>,
    @location(7) stroke_width: f32,
    @location(8) stroke_opacity: f32,
};

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    let drawable = drawableVector[globalIndex.value];

    let radius = unpack_mix_float(in.radius, drawable.radius_t);
    let stroke_width = unpack_mix_float(in.stroke_width, drawable.stroke_width_t);

    let pos_f = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let extrude = glMod2v(pos_f, vec2<f32>(2.0, 2.0)) * 2.0 - vec2<f32>(1.0, 1.0);
    let scaled_extrude = extrude * drawable.extrude_scale;
    let circle_center = floor(pos_f * 0.5);
    let world_pos = circle_center + scaled_extrude * (radius + stroke_width);

    out.position = drawable.matrix * vec4<f32>(world_pos, 0.0, 1.0);
    out.antialiasblur = 1.0 / (radius + stroke_width);
    out.extrude = extrude;
    out.color = unpack_mix_color(in.color, drawable.color_t);
    out.radius = radius;
    out.blur = unpack_mix_float(in.blur, drawable.blur_t);
    out.opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
    out.stroke_color = unpack_mix_color(in.stroke_color, drawable.stroke_color_t);
    out.stroke_width = stroke_width;
    out.stroke_opacity = unpack_mix_float(in.stroke_opacity, drawable.stroke_opacity_t);
    
    return out;
}
)";
    
    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) extrude: vec2<f32>,
    @location(1) antialiasblur: f32,
    @location(2) color: vec4<f32>,
    @location(3) radius: f32,
    @location(4) blur: f32,
    @location(5) opacity: f32,
    @location(6) stroke_color: vec4<f32>,
    @location(7) stroke_width: f32,
    @location(8) stroke_opacity: f32,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let extrude_length = length(in.extrude);
    let antialiased_blur = -max(in.blur, in.antialiasblur);
    
    let opacity_t = smoothstep(0.0, antialiased_blur, extrude_length - 1.0);
    
    var color_t: f32;
    if (in.stroke_width < 0.01) {
        color_t = 0.0;
    } else {
        color_t = smoothstep(antialiased_blur, 0.0, extrude_length - in.radius / (in.radius + in.stroke_width));
    }
    
    let final_color = mix(in.color * in.opacity, in.stroke_color * in.stroke_opacity, color_t);
    return opacity_t * final_color;
}
)";
};

} // namespace shaders
} // namespace mbgl
