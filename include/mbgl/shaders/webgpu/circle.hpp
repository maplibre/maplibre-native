#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "CircleShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    
    static constexpr const char* vertex = R"(
struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) extrude: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) extrude: vec2<f32>,
};

struct CircleUBO {
    matrix: mat4x4<f32>,
    extrude_scale: vec2<f32>,
    camera_to_center_distance: f32,
    pitch_with_map: f32,
};

@group(0) @binding(0) var<uniform> circle_ubo: CircleUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    // Multiply the extrude by the radius and add to position
    let circle_center = in.position;
    let corner_position = circle_center + in.extrude * circle_ubo.extrude_scale;
    
    out.position = circle_ubo.matrix * vec4<f32>(corner_position, 0.0, 1.0);
    out.extrude = in.extrude;
    
    return out;
}
)";
    
    static constexpr const char* fragment = R"(
struct FragmentInput {
    @location(0) extrude: vec2<f32>,
};

struct CirclePropsUBO {
    color: vec4<f32>,
    radius: f32,
    blur: f32,
    opacity: f32,
    stroke_color: vec4<f32>,
    stroke_width: f32,
    stroke_opacity: f32,
    pad1: f32,
    pad2: f32,
};

@group(0) @binding(1) var<uniform> circle_props: CirclePropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let extrude_length = length(in.extrude);
    let antialiased_blur = -max(circle_props.blur, 1.0 / 32.0);
    
    let opacity_t = smoothstep(0.0, antialiased_blur, extrude_length - 1.0);
    
    // Determine if we're in the stroke or fill
    let stroke_width = circle_props.stroke_width;
    let stroke_t = smoothstep(
        antialiased_blur,
        0.0,
        abs(extrude_length - 1.0 + stroke_width / 2.0) - stroke_width / 2.0
    );
    
    // Mix stroke and fill colors
    var color: vec4<f32>;
    if (stroke_width > 0.0 && stroke_t > 0.0) {
        color = mix(
            vec4<f32>(circle_props.color.rgb, circle_props.color.a * circle_props.opacity),
            vec4<f32>(circle_props.stroke_color.rgb, circle_props.stroke_color.a * circle_props.stroke_opacity),
            stroke_t
        );
    } else {
        color = vec4<f32>(circle_props.color.rgb, circle_props.color.a * circle_props.opacity);
    }
    
    return vec4<f32>(color.rgb, color.a * (1.0 - opacity_t));
}
)";
};

} // namespace shaders
} // namespace mbgl