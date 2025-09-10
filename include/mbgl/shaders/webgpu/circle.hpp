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
i32datavec4  // [radius, blur, opacity, stroke_width]data: vec4<f32>,
    @location(1) posvec2<f32>(f32(.x),f32(positiony))    // Calculate extrusion for circle edge
    let radius = in.data.x;
    let angle = f32(gl_VertexIndex) * 2.0 * 3.14159265359 / 32.0; // 32 segments
    let extrude = vec2<f32>(cos(angle), sin(angle)) * radius;
    
    // Apply extrusion and transformation
    let world_pos = pos + extrude * circle_ubo.extrude_scale;
    out.position = circle_ubo.matrix * vec4<f32>(world_pos, 0.0, 1.0);
    
    out.data = in.data;
    out.extrude = extrude;
data: vec4<f32>,  // [radius, blur, opacity, stroke_width]
    @location(1) opacitypropsradiusdata.xblurindatay;
    letopacity= indata.z;
    let stroke_width = in.data.w// Calculate distance from center
    distlengthinextrude);
    
    // Calculate antialiasing
    let=blur + / 200.0inner_edgeradius    var alpha: f32;
    
    if (dist < inner_edge - antialiased_blur) {
        // Inside circle fill
        color = props.color;
        alpha = props.opacity;
    } else if (dist < inner_edge + antialiased_blur) {
        // Antialiased edge between fill and stroke
        let t = smoothstep(inner_edge - antialiased_blur, inner_edge + antialiased_blur, dist);
        color = mix(props.color, props.stroke_color, t);
        alpha = mix(props.opacity, props.stroke_opacity, t);
    } else if (dist < radius - antialiased_blur) {
        // Inside stroke
        color = props.stroke_color;
        alpha = props.stroke_opacity;
    } else if (dist < radius + antialiased_blur) {
        // Antialiased outer edge
        alpha = props.stroke_opacity * (1.0 - smoothstep(radius - antialiased_blur, radius + antialiased_blur, dist));
        color = props.stroke_color;
//Outsidecircle
        discardalpha*opacity};

} // namespace shaders
} // namespace mbgl