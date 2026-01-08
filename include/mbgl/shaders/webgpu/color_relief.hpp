#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "ColorReliefShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 3> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<i32>,
    @location(6) texture_pos: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) frag_position: vec2<f32>,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct ColorReliefDrawableUBO {
    matrix: mat4x4<f32>,
};

struct ColorReliefTilePropsUBO {
    unpack: vec4<f32>,
    dimension: vec2<f32>,
    color_ramp_size: i32,
    pad_tile0: f32,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<ColorReliefDrawableUBO>;
@group(0) @binding(4) var<storage, read> tilePropsVector: array<ColorReliefTilePropsUBO>;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value];
    let tileProps = tilePropsVector[globalIndex.value];

    out.position = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);

    let a_pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let epsilon = vec2<f32>(1.0, 1.0) / tileProps.dimension;
    let scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    out.frag_position = (a_pos / 8192.0) * scale + epsilon;

    // Handle poles
    out.frag_position.y = select(out.frag_position.y, 0.0, a_pos.y < -32767.5);
    out.frag_position.y = select(out.frag_position.y, 1.0, a_pos.y > 32766.5);

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) frag_position: vec2<f32>,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct ColorReliefTilePropsUBO {
    unpack: vec4<f32>,
    dimension: vec2<f32>,
    color_ramp_size: i32,
    pad_tile0: f32,
};

struct ColorReliefEvaluatedPropsUBO {
    opacity: f32,
    pad_eval0: f32,
    pad_eval1: f32,
    pad_eval2: f32,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(4) var<storage, read> tilePropsVector: array<ColorReliefTilePropsUBO>;
@group(0) @binding(5) var<uniform> props: ColorReliefEvaluatedPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var dem_texture: texture_2d<f32>;
@group(1) @binding(2) var elevation_stops_texture: texture_2d<f32>;
@group(1) @binding(3) var color_stops_texture: texture_2d<f32>;

fn getElevation(coord: vec2<f32>, unpack: vec4<f32>) -> f32 {
    var data = textureSample(dem_texture, texture_sampler, coord) * 255.0;
    data.a = -1.0;
    return dot(data, unpack);
}

fn getElevationStop(stop: i32, color_ramp_size: i32) -> f32 {
    // Use textureLoad for RGBA32Float texture (not filterable in WebGPU)
    return textureLoad(elevation_stops_texture, vec2<i32>(stop, 0), 0).r;
}

fn getColorStop(stop: i32, color_ramp_size: i32) -> vec4<f32> {
    let x = (f32(stop) + 0.5) / f32(color_ramp_size);
    return textureSample(color_stops_texture, texture_sampler, vec2<f32>(x, 0.5));
}

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
#ifdef OVERDRAW_INSPECTOR
    return vec4<f32>(1.0, 1.0, 1.0, 1.0);
#endif

    let tileProps = tilePropsVector[globalIndex.value];
    let el = getElevation(in.frag_position, tileProps.unpack);

    // Binary search to find surrounding elevation stops
    var r: i32 = tileProps.color_ramp_size - 1;
    var l: i32 = 0;

    loop {
        if (r - l <= 1) { break; }
        let m = (r + l) / 2;
        let el_m = getElevationStop(m, tileProps.color_ramp_size);
        if (el < el_m) {
            r = m;
        } else {
            l = m;
        }
    }

    // Get elevation values and colors at the stops
    let el_l = getElevationStop(l, tileProps.color_ramp_size);
    let el_r = getElevationStop(r, tileProps.color_ramp_size);

    let color_l = getColorStop(l, tileProps.color_ramp_size);
    let color_r = getColorStop(r, tileProps.color_ramp_size);

    // Interpolate color based on elevation
    // Guard against division by zero when el_r == el_l
    let denom = el_r - el_l;
    var t: f32 = 0.0;
    if (abs(denom) >= 0.0001) {
        t = clamp((el - el_l) / denom, 0.0, 1.0);
    }
    let final_color = mix(color_l, color_r, t) * props.opacity;

    return vec4<f32>(final_color);
}
)";
};

} // namespace shaders
} // namespace mbgl
