#pragma once

#include <mbgl/shaders/collision_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "CollisionBoxShader";
    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec2<i32>,
    @location(4) anchor_position: vec2<i32>,
    @location(5) extrude: vec2<i32>,
    @location(6) placed: vec2<u32>,
    @location(7) shift: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) placed: f32,
    @location(1) not_used: f32,
};

struct CollisionDrawableUBO {
    matrix: mat4x4<f32>,
};

struct CollisionTilePropsUBO {
    extrude_scale: vec2<f32>,
    overscale_factor: f32,
    pad1: f32,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<uniform> drawable: CollisionDrawableUBO;
@group(0) @binding(4) var<uniform> tile_props: CollisionTilePropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let projected_point = drawable.matrix * vec4<f32>(f32(in.anchor_position.x), f32(in.anchor_position.y), 0.0, 1.0);
    let camera_to_anchor_distance = projected_point.w;

    let collision_perspective_ratio = clamp(
        0.5 + 0.5 * (paintParams.camera_to_center_distance / camera_to_anchor_distance),
        0.0,
        4.0);

    out.position = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    let extrude_shift = vec2<f32>(f32(in.extrude.x), f32(in.extrude.y)) + in.shift;
    out.position.x += extrude_shift.x * tile_props.extrude_scale.x * out.position.w * collision_perspective_ratio;
    out.position.y += extrude_shift.y * tile_props.extrude_scale.y * out.position.w * collision_perspective_ratio;

    out.placed = f32(in.placed.x);
    out.not_used = f32(in.placed.y);

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) placed: f32,
    @location(1) not_used: f32,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let alpha = 0.5;

    // Red = collision, hide label
    var color = vec4<f32>(1.0, 0.0, 0.0, 1.0) * alpha;

    // Blue = no collision, label is showing
    if (in.placed > 0.5) {
        color = vec4<f32>(0.0, 0.0, 1.0, 0.5) * alpha;
    }

    if (in.not_used > 0.5) {
        // This box not used, fade it out
        color = color * 0.1;
    }

    return color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "CollisionCircleShader";
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec2<i32>,
    @location(4) anchor_position: vec2<i32>,
    @location(5) extrude: vec2<i32>,
    @location(6) placed: vec2<u32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) placed: f32,
    @location(1) not_used: f32,
    @location(2) radius: f32,
    @location(3) extrude: vec2<f32>,
    @location(4) extrude_scale: vec2<f32>,
};

struct GlobalPaintParamsUBO {
    pattern_atlas_texsize: vec2<f32>,
    units_to_pixels: vec2<f32>,
    world_size: vec2<f32>,
    camera_to_center_distance: f32,
    symbol_fade_change: f32,
    aspect_ratio: f32,
    pixel_ratio: f32,
    map_zoom: f32,
    pad1: f32,
};

struct CollisionDrawableUBO {
    matrix: mat4x4<f32>,
};

struct CollisionTilePropsUBO {
    extrude_scale: vec2<f32>,
    overscale_factor: f32,
    pad1: f32,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<uniform> drawable: CollisionDrawableUBO;
@group(0) @binding(4) var<uniform> tile_props: CollisionTilePropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;



    let projected_point = drawable.matrix * vec4<f32>(f32(in.anchor_position.x), f32(in.anchor_position.y), 0.0, 1.0);
    let camera_to_anchor_distance = projected_point.w;

    let perspective = max(camera_to_anchor_distance, 1e-6);
    let collision_perspective_ratio = clamp(
        0.5 + 0.5 * (paintParams.camera_to_center_distance / perspective),
        0.0,
        4.0);

    var position = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);
    let padding_factor = 1.2;
    let extrude_vec = vec2<f32>(f32(in.extrude.x), f32(in.extrude.y));
    position.x += extrude_vec.x * tile_props.extrude_scale.x * padding_factor * position.w * collision_perspective_ratio;
    position.y += extrude_vec.y * tile_props.extrude_scale.y * padding_factor * position.w * collision_perspective_ratio;

    out.position = position;

    out.placed = f32(in.placed.x);
    out.not_used = f32(in.placed.y);
    out.radius = abs(f32(in.extrude.y));
    out.extrude = extrude_vec * padding_factor;
    out.extrude_scale = tile_props.extrude_scale * paintParams.camera_to_center_distance * collision_perspective_ratio;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) placed: f32,
    @location(1) not_used: f32,
    @location(2) radius: f32,
    @location(3) extrude: vec2<f32>,
    @location(4) extrude_scale: vec2<f32>,
};

struct CollisionTilePropsUBO {
    extrude_scale: vec2<f32>,
    overscale_factor: f32,
    pad1: f32,
};

@group(0) @binding(4) var<uniform> tile_props: CollisionTilePropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let alpha = 0.5;

    // Red = collision, hide label
    var color = vec4<f32>(1.0, 0.0, 0.0, 1.0) * alpha;

    // Blue = no collision, label is showing
    if (in.placed > 0.5) {
        color = vec4<f32>(0.0, 0.0, 1.0, 0.5) * alpha;
    }

    if (in.not_used > 0.5) {
        color = color * 0.2;
    }

    // Add circle outline effect
    let extrude_scale_length = max(length(in.extrude_scale), 1e-6);
    let extrude_length = length(in.extrude) * extrude_scale_length;
    let overscale = max(tile_props.overscale_factor, 1.0);
    let stroke_width = 15.0 * extrude_scale_length / overscale;
    let radius = in.radius * extrude_scale_length;
    let distance_to_edge = abs(extrude_length - radius);
    let opacity_t = smoothstep(-stroke_width, 0.0, -distance_to_edge);

    return opacity_t * color;
}
)";
};

} // namespace shaders
} // namespace mbgl
