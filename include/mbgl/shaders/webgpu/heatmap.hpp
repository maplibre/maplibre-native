#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "HeatmapShader";
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
#ifndef HAS_UNIFORM_u_weight
    @location(1) weight: vec2<f32>,
#endif
#ifndef HAS_UNIFORM_u_radius
    @location(2) radius: vec2<f32>,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) weight: f32,
    @location(1) extrude: vec2<f32>,
};

struct HeatmapDrawableUBO {
    matrix: mat4x4<f32>,
    extrude_scale: f32,
    weight_t: f32,
    radius_t: f32,
    pad1: f32,
};

struct HeatmapEvaluatedPropsUBO {
    weight: f32,
    radius: f32,
    intensity: f32,
    padding: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad: vec3<u32>,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<HeatmapDrawableUBO>;
@group(0) @binding(4) var<uniform> props: HeatmapEvaluatedPropsUBO;

const ZERO: f32 = 1.0 / 255.0 / 16.0;
const GAUSS_COEF: f32 = 0.3989422804014327;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;

    let drawable = drawableVector[globalIndex.value];

#ifdef HAS_UNIFORM_u_weight
    let weight = props.weight;
#else
    let weight = unpack_mix_float(in.weight, drawable.weight_t);
#endif

#ifdef HAS_UNIFORM_u_radius
    let radius = props.radius;
#else
    let radius = unpack_mix_float(in.radius, drawable.radius_t);
#endif

    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let unscaled_extrude = fma(vec2<f32>(f32(in.position.x & 1), f32(in.position.y & 1)), vec2<f32>(2.0, 2.0),
                               vec2<f32>(-1.0, -1.0));

    let S = sqrt(-2.0 * log(ZERO / (max(weight, ZERO) * max(props.intensity, ZERO) * GAUSS_COEF))) / 3.0;
    let extrude = S * unscaled_extrude;
    let scaled_extrude = extrude * radius * drawable.extrude_scale;

    let base = floor(pos * 0.5);
    out.position = drawable.matrix * vec4<f32>(base + scaled_extrude, 0.0, 1.0);
    out.weight = weight;
    out.extrude = extrude;

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) weight: f32,
    @location(1) extrude: vec2<f32>,
};

struct HeatmapEvaluatedPropsUBO {
    weight: f32,
    radius: f32,
    intensity: f32,
    padding: f32,
};

const GAUSS_COEF: f32 = 0.3989422804014327;

@group(0) @binding(4) var<uniform> props: HeatmapEvaluatedPropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
#ifdef OVERDRAW_INSPECTOR
    return vec4<f32>(1.0, 1.0, 1.0, 1.0);
#else
    let d = -0.5 * 3.0 * 3.0 * dot(in.extrude, in.extrude);
    let val = in.weight * props.intensity * GAUSS_COEF * exp(d);
    return vec4<f32>(val, 1.0, 1.0, 1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
