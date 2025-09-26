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
    scale_with_map: i32,
    pitch_with_map: i32,
    expression_mask: f32,
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

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

const CIRCLE_EXPRESSION_COLOR: u32 = 1u << 0u;
const CIRCLE_EXPRESSION_RADIUS: u32 = 1u << 1u;
const CIRCLE_EXPRESSION_BLUR: u32 = 1u << 2u;
const CIRCLE_EXPRESSION_OPACITY: u32 = 1u << 3u;
const CIRCLE_EXPRESSION_STROKE_COLOR: u32 = 1u << 4u;
const CIRCLE_EXPRESSION_STROKE_WIDTH: u32 = 1u << 5u;
const CIRCLE_EXPRESSION_STROKE_OPACITY: u32 = 1u << 6u;

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
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
    @location(1) circle_data: vec4<f32>,
    @location(2) color: vec4<f32>,
    @location(3) stroke_color: vec4<f32>,
    @location(4) stroke_data: vec2<f32>,
};

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let drawable = drawableVector[globalIndex.value];
    let expression_mask = bitcast<u32>(props.expression_mask);
    let scale_with_map = props.scale_with_map != 0;
    let pitch_with_map = props.pitch_with_map != 0;

    let pos_f = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let extrude = glMod2v(pos_f, vec2<f32>(2.0, 2.0)) * 2.0 - vec2<f32>(1.0, 1.0);
    let scaled_extrude = extrude * drawable.extrude_scale;
    let circle_center = floor(pos_f * 0.5);

    var color = props.color;
    if ((expression_mask & CIRCLE_EXPRESSION_COLOR) != 0u) {
        color = unpack_mix_color(in.color, drawable.color_t);
    }

    var radius = props.radius;
    if ((expression_mask & CIRCLE_EXPRESSION_RADIUS) != 0u) {
        radius = unpack_mix_float(in.radius, drawable.radius_t);
    }

    var blur = props.blur;
    if ((expression_mask & CIRCLE_EXPRESSION_BLUR) != 0u) {
        blur = unpack_mix_float(in.blur, drawable.blur_t);
    }

    var opacity = props.opacity;
    if ((expression_mask & CIRCLE_EXPRESSION_OPACITY) != 0u) {
        opacity = unpack_mix_float(in.opacity, drawable.opacity_t);
    }

    var stroke_color = props.stroke_color;
    if ((expression_mask & CIRCLE_EXPRESSION_STROKE_COLOR) != 0u) {
        stroke_color = unpack_mix_color(in.stroke_color, drawable.stroke_color_t);
    }

    var stroke_width = props.stroke_width;
    if ((expression_mask & CIRCLE_EXPRESSION_STROKE_WIDTH) != 0u) {
        stroke_width = unpack_mix_float(in.stroke_width, drawable.stroke_width_t);
    }

    var stroke_opacity = props.stroke_opacity;
    if ((expression_mask & CIRCLE_EXPRESSION_STROKE_OPACITY) != 0u) {
        stroke_opacity = unpack_mix_float(in.stroke_opacity, drawable.stroke_opacity_t);
    }

    let radius_with_stroke = radius + stroke_width;

    var position: vec4<f32>;
    if (pitch_with_map) {
        var corner_position = circle_center;
        if (scale_with_map) {
            corner_position += scaled_extrude * radius_with_stroke;
        } else {
            let projected_center = drawable.matrix * vec4<f32>(circle_center, 0.0, 1.0);
            corner_position += scaled_extrude * radius_with_stroke *
                               (projected_center.w / paintParams.camera_to_center_distance);
        }
        position = drawable.matrix * vec4<f32>(corner_position, 0.0, 1.0);
    } else {
        position = drawable.matrix * vec4<f32>(circle_center, 0.0, 1.0);
        var factor = position.w;
        if (scale_with_map) {
            factor = paintParams.camera_to_center_distance;
        }
        let delta = scaled_extrude * radius_with_stroke * factor;
        position = vec4<f32>(position.x + delta.x, position.y + delta.y, position.z, position.w);
    }

    let antialiasblur = 1.0 / max(paintParams.pixel_ratio * radius_with_stroke, 1e-6);

    out.position = position;
    out.extrude = extrude;
    out.circle_data = vec4<f32>(antialiasblur, radius, blur, opacity);
    out.color = color;
    out.stroke_color = stroke_color;
    out.stroke_data = vec2<f32>(stroke_width, stroke_opacity);

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) extrude: vec2<f32>,
    @location(1) circle_data: vec4<f32>,
    @location(2) color: vec4<f32>,
    @location(3) stroke_color: vec4<f32>,
    @location(4) stroke_data: vec2<f32>,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let extrude_length = length(in.extrude);
    let antialiasblur = in.circle_data.x;
    let radius = in.circle_data.y;
    let blur = in.circle_data.z;
    let opacity = in.circle_data.w;
    let stroke_width = in.stroke_data.x;
    let stroke_opacity = in.stroke_data.y;
    let antialiased_blur = -max(blur, antialiasblur);

    let opacity_t = smoothstep(0.0, antialiased_blur, extrude_length - 1.0);

    var color_t: f32;
    if (stroke_width < 0.01) {
        color_t = 0.0;
    } else {
        color_t = smoothstep(antialiased_blur, 0.0, extrude_length - radius / (radius + stroke_width));
    }

    let final_color = mix(in.color * opacity, in.stroke_color * stroke_opacity, color_t);
    return opacity_t * final_color;
}
)";
};

} // namespace shaders
} // namespace mbgl
