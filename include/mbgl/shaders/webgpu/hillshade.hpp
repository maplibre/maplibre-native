#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "HillshadeShader";
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

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct HillshadeDrawableUBO {
    matrix: mat4x4<f32>,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<HillshadeDrawableUBO>;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value];
    out.position = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), 0.0, 1.0);

    var tex = vec2<f32>(f32(in.texcoord.x), f32(in.texcoord.y)) / 8192.0;
    tex.y = 1.0 - tex.y;
    out.tex_coord = tex;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) tex_coord: vec2<f32>,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct HillshadeTilePropsUBO {
    latrange: vec2<f32>,
    light: vec2<f32>,
};

struct HillshadeEvaluatedPropsUBO {
    highlight: vec4<f32>,
    shadow: vec4<f32>,
    accent: vec4<f32>,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<HillshadeTilePropsUBO>;
@group(0) @binding(4) var<uniform> props: HillshadeEvaluatedPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var hillshade_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
#ifdef OVERDRAW_INSPECTOR
    return vec4<f32>(1.0, 1.0, 1.0, 1.0);
#endif

    let tileProps = tilePropsVector[globalIndex.value];
    let pixel = textureSample(hillshade_texture, texture_sampler, in.tex_coord);

    let deriv = pixel.rg * 2.0 - vec2<f32>(1.0, 1.0);

    let latRange = tileProps.latrange;
    let latitude = (latRange.x - latRange.y) * in.tex_coord.y + latRange.y;
    let scaleFactor = cos(radians(latitude));
    let slope = atan(1.25 * length(deriv) / scaleFactor);
    let aspectDefault = 0.5 * PI * select(-1.0, 1.0, deriv.y > 0.0);
    let aspect = select(aspectDefault, atan2(deriv.y, -deriv.x), deriv.x != 0.0);

    let intensity = tileProps.light.x;
    let azimuth = tileProps.light.y + PI;

    let base = 1.875 - intensity * 1.75;
    let maxValue = 0.5 * PI;
    let denom = pow(base, maxValue) - 1.0;
    let useLinear = abs(intensity - 0.5) < 1e-6;
    let scaledSlope = select(((pow(base, slope) - 1.0) / denom) * maxValue,
                             slope,
                             useLinear);

    let accent = cos(scaledSlope);
    let accentColor = (1.0 - accent) * props.accent * clamp(intensity * 2.0, 0.0, 1.0);

    let shade = abs(glMod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);
    let shadeColor = mix(props.shadow, props.highlight, shade) * sin(scaledSlope) * clamp(intensity * 2.0, 0.0, 1.0);

    let color = accentColor * (1.0 - shadeColor.a) + shadeColor;
    return color;
}
)";
};

} // namespace shaders
} // namespace mbgl
