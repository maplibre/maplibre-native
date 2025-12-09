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
const PI: f32 = 3.141592653589793;
const STANDARD: i32 = 0;
const COMBINED: i32 = 1;
const IGOR: i32 = 2;
const MULTIDIRECTIONAL: i32 = 3;
const BASIC: i32 = 4;

struct FragmentInput {
    @location(0) tex_coord: vec2<f32>,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

struct HillshadeTilePropsUBO {
    latrange: vec2<f32>,
    exaggeration: f32,
    method: i32,
    num_lights: i32,
    pad0: f32,
    pad1: f32,
    pad2: f32,
};

struct HillshadeEvaluatedPropsUBO {
    accent: vec4<f32>,
    altitudes: vec4<f32>,
    azimuths: vec4<f32>,
    shadows: array<vec4<f32>, 4>,
    highlights: array<vec4<f32>, 4>,
};

@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(0) @binding(3) var<storage, read> tilePropsVector: array<HillshadeTilePropsUBO>;
@group(0) @binding(4) var<uniform> props: HillshadeEvaluatedPropsUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var hillshade_texture: texture_2d<f32>;

fn get_aspect(deriv: vec2<f32>) -> f32 {
    let aspectDefault = 0.5 * PI * select(-1.0, 1.0, deriv.y > 0.0);
    return select(aspectDefault, atan2(deriv.y, -deriv.x), deriv.x != 0.0);
}

fn standard_hillshade(deriv: vec2<f32>, tileProps: HillshadeTilePropsUBO) -> vec4<f32> {
    let azimuth = props.azimuths.x + PI;
    let slope = atan(0.625 * length(deriv));
    let aspect = get_aspect(deriv);

    let intensity = tileProps.exaggeration;

    let base = 1.875 - intensity * 1.75;
    let maxValue = 0.5 * PI;
    let denom = pow(base, maxValue) - 1.0;
    let useLinear = abs(intensity - 0.5) < 1e-6;
    let scaledSlope = select(((pow(base, slope) - 1.0) / denom) * maxValue, slope, useLinear);

    let accent = cos(scaledSlope);
    let accentColor = (1.0 - accent) * props.accent * clamp(intensity * 2.0, 0.0, 1.0);

    let shade = abs(glMod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);
    let shadeColor = mix(props.shadows[0], props.highlights[0], shade) * sin(scaledSlope) * clamp(intensity * 2.0, 0.0, 1.0);

    return accentColor * (1.0 - shadeColor.a) + shadeColor;
}

fn basic_hillshade(deriv: vec2<f32>, tileProps: HillshadeTilePropsUBO) -> vec4<f32> {
    let scaled_deriv = deriv * tileProps.exaggeration * 2.0;
    let azimuth = props.azimuths.x + PI;
    let cos_az = cos(azimuth);
    let sin_az = sin(azimuth);
    let cos_alt = cos(props.altitudes.x);
    let sin_alt = sin(props.altitudes.x);

    let cang = (sin_alt - (scaled_deriv.y * cos_az * cos_alt - scaled_deriv.x * sin_az * cos_alt)) / sqrt(1.0 + dot(scaled_deriv, scaled_deriv));
    let shade = clamp(cang, 0.0, 1.0);

    return select(props.shadows[0] * (1.0 - 2.0 * shade),
                  props.highlights[0] * (2.0 * shade - 1.0),
                  shade > 0.5);
}

fn multidirectional_hillshade(deriv: vec2<f32>, tileProps: HillshadeTilePropsUBO) -> vec4<f32> {
    let scaled_deriv = deriv * tileProps.exaggeration * 2.0;
    var total_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);

    let num_lights = min(tileProps.num_lights, 4);

    for (var i: i32 = 0; i < num_lights; i = i + 1) {
        let altitude = select(select(select(props.altitudes.x, props.altitudes.y, i == 1), props.altitudes.z, i == 2), props.altitudes.w, i == 3);
        let azimuth = select(select(select(props.azimuths.x, props.azimuths.y, i == 1), props.azimuths.z, i == 2), props.azimuths.w, i == 3);

        let cos_alt = cos(altitude);
        let sin_alt = sin(altitude);
        let cos_az = -cos(azimuth);
        let sin_az = -sin(azimuth);

        let cang = (sin_alt - (scaled_deriv.y * cos_az * cos_alt - scaled_deriv.x * sin_az * cos_alt)) / sqrt(1.0 + dot(scaled_deriv, scaled_deriv));
        let shade = clamp(cang, 0.0, 1.0);

        if (shade > 0.5) {
            total_color = total_color + props.highlights[i] * (2.0 * shade - 1.0) / f32(num_lights);
        } else {
            total_color = total_color + props.shadows[i] * (1.0 - 2.0 * shade) / f32(num_lights);
        }
    }

    return total_color;
}

fn combined_hillshade(deriv: vec2<f32>, tileProps: HillshadeTilePropsUBO) -> vec4<f32> {
    let scaled_deriv = deriv * tileProps.exaggeration * 2.0;
    let azimuth = props.azimuths.x + PI;
    let cos_az = cos(azimuth);
    let sin_az = sin(azimuth);
    let cos_alt = cos(props.altitudes.x);
    let sin_alt = sin(props.altitudes.x);

    let cang = acos((sin_alt - (scaled_deriv.y * cos_az * cos_alt - scaled_deriv.x * sin_az * cos_alt)) / sqrt(1.0 + dot(scaled_deriv, scaled_deriv)));
    let clamped_cang = clamp(cang, 0.0, PI / 2.0);

    let shade = clamped_cang * atan(length(scaled_deriv)) * 4.0 / PI / PI;
    let highlight = (PI / 2.0 - clamped_cang) * atan(length(scaled_deriv)) * 4.0 / PI / PI;

    return props.shadows[0] * shade + props.highlights[0] * highlight;
}

fn igor_hillshade(deriv: vec2<f32>, tileProps: HillshadeTilePropsUBO) -> vec4<f32> {
    let scaled_deriv = deriv * tileProps.exaggeration * 2.0;
    let aspect = get_aspect(scaled_deriv);
    let azimuth = props.azimuths.x + PI;

    let slope_strength = atan(length(scaled_deriv)) * 2.0 / PI;
    let aspect_strength = 1.0 - abs(glMod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);

    let shadow_strength = slope_strength * aspect_strength;
    let highlight_strength = slope_strength * (1.0 - aspect_strength);

    return props.shadows[0] * shadow_strength + props.highlights[0] * highlight_strength;
}

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
#ifdef OVERDRAW_INSPECTOR
    return vec4<f32>(1.0, 1.0, 1.0, 1.0);
#endif

    let tileProps = tilePropsVector[globalIndex.value];
    let pixel = textureSample(hillshade_texture, texture_sampler, in.tex_coord);

    let latRange = tileProps.latrange;
    let latitude = (latRange.x - latRange.y) * in.tex_coord.y + latRange.y;
    let scaleFactor = cos(radians(latitude));

    let deriv = ((pixel.rg * 8.0) - vec2<f32>(4.0, 4.0)) / scaleFactor;

    if (tileProps.method == BASIC) {
        return basic_hillshade(deriv, tileProps);
    } else if (tileProps.method == COMBINED) {
        return combined_hillshade(deriv, tileProps);
    } else if (tileProps.method == IGOR) {
        return igor_hillshade(deriv, tileProps);
    } else if (tileProps.method == MULTIDIRECTIONAL) {
        return multidirectional_hillshade(deriv, tileProps);
    } else {
        return standard_hillshade(deriv, tileProps);
    }
}
)";
};

} // namespace shaders
} // namespace mbgl

