#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "HillshadePrepareShader";
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(5) position: vec2<i32>,
    @location(6) texcoord: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) tex_coord: vec2<f32>,
};

struct HillshadePrepareDrawableUBO {
    matrix: mat4x4<f32>,
};

struct HillshadePrepareTilePropsUBO {
    unpack: vec4<f32>,
    dimension: vec2<f32>,
    zoom: f32,
    maxzoom: f32,
};

@group(0) @binding(2) var<uniform> drawable: HillshadePrepareDrawableUBO;
@group(0) @binding(4) var<uniform> tileProps: HillshadePrepareTilePropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);

    let epsilon = vec2<f32>(1.0, 1.0) / tileProps.dimension;
    let scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    out.tex_coord = vec2<f32>(f32(in.texcoord.x), f32(in.texcoord.y)) / 8192.0 * scale + epsilon;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex_coord: vec2<f32>,
};

struct HillshadePrepareTilePropsUBO {
    unpack: vec4<f32>,
    dimension: vec2<f32>,
    zoom: f32,
    maxzoom: f32,
};

@group(0) @binding(4) var<uniform> tileProps: HillshadePrepareTilePropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var dem_texture: texture_2d<f32>;

fn getElevation(coord: vec2<f32>) -> f32 {
    var data = textureSample(dem_texture, texture_sampler, coord) * 255.0;
    data.a = -1.0;
    return dot(data, tileProps.unpack) / 4.0;
}

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
#ifdef OVERDRAW_INSPECTOR
    return vec4<f32>(1.0, 1.0, 1.0, 1.0);
#endif

    let epsilon = vec2<f32>(1.0, 1.0) / tileProps.dimension;

    let a = getElevation(in.tex_coord + vec2<f32>(-epsilon.x, -epsilon.y));
    let b = getElevation(in.tex_coord + vec2<f32>(0.0, -epsilon.y));
    let c = getElevation(in.tex_coord + vec2<f32>(epsilon.x, -epsilon.y));
    let d = getElevation(in.tex_coord + vec2<f32>(-epsilon.x, 0.0));
    let f = getElevation(in.tex_coord + vec2<f32>(epsilon.x, 0.0));
    let g = getElevation(in.tex_coord + vec2<f32>(-epsilon.x, epsilon.y));
    let h = getElevation(in.tex_coord + vec2<f32>(0.0, epsilon.y));
    let i = getElevation(in.tex_coord + vec2<f32>(epsilon.x, epsilon.y));

    var exaggeration = 0.3;
    if (tileProps.zoom < 4.5) {
        exaggeration = 0.35;
    }
    if (tileProps.zoom < 2.0) {
        exaggeration = 0.4;
    }

    let denom = pow(2.0, (tileProps.zoom - tileProps.maxzoom) * exaggeration + 19.2562 - tileProps.zoom);
    let deriv = vec2<f32>((c + f + f + i) - (a + d + d + g),
                          (g + h + h + i) - (a + b + b + c)) / denom;

    let color = clamp(vec4<f32>(deriv.x / 2.0 + 0.5,
                                 deriv.y / 2.0 + 0.5,
                                 1.0,
                                 1.0),
                      vec4<f32>(0.0, 0.0, 0.0, 0.0),
                      vec4<f32>(1.0, 1.0, 1.0, 1.0));
    return color;
}
)";
};

} // namespace shaders
} // namespace mbgl
