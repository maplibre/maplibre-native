#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/webgpu/shader_program.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::SkyShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "SkyShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) pos: vec2<f32>,
};

@vertex
fn main(in: VertexInput) -> VertexOutput {
    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    var out: VertexOutput;
    out.position = vec4<f32>(pos, 1.0, 1.0);
    out.pos = pos;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) pos: vec2<f32>,
};

struct SkyPropsUBO {
    sky_color: vec4<f32>,
    horizon_color: vec4<f32>,
    horizon: vec2<f32>,
    horizon_normal: vec2<f32>,
    viewport_size: vec2<f32>,
    sky_horizon_blend: f32,
    sky_blend: f32,
};

@group(0) @binding(5) var<uniform> sky: SkyPropsUBO;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let pixel = (in.pos * 0.5 + 0.5) * sky.viewport_size;
    let distance_to_horizon = dot(pixel - sky.horizon, sky.horizon_normal);
    var color = vec4<f32>(0.0);
    if (distance_to_horizon > 0.0) {
        if (sky.sky_horizon_blend > 0.0 && distance_to_horizon < sky.sky_horizon_blend) {
            let blend = 1.0 - distance_to_horizon / sky.sky_horizon_blend;
            color = mix(sky.sky_color, sky.horizon_color, blend * blend);
        } else {
            color = sky.sky_color;
        }
    }
    return color * (1.0 - sky.sky_blend);
}
)";
};

template <>
struct ShaderSource<BuiltIn::AtmosphereShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "AtmosphereShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(0) position: vec2<i32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) view_direction: vec3<f32>,
};

struct AtmospherePropsUBO {
    inv_view_projection: mat4x4<f32>,
    camera_position: vec4<f32>,
    sun_position: vec4<f32>,
};

@group(0) @binding(5) var<uniform> atmosphere: AtmospherePropsUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    let pos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    let target = atmosphere.inv_view_projection * vec4<f32>(pos, 0.0, 1.0);
    var out: VertexOutput;
    // WebGPU uses a zero-to-one NDC depth range, so 0.5 matches WebGL's
    // framebuffer depth for clip-space z=0.
    out.position = vec4<f32>(pos, 0.5, 1.0);
    out.view_direction = target.xyz / target.w - atmosphere.camera_position.xyz;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) view_direction: vec3<f32>,
};

struct AtmospherePropsUBO {
    inv_view_projection: mat4x4<f32>,
    camera_position: vec4<f32>,
    sun_position: vec4<f32>,
};

@group(0) @binding(5) var<uniform> atmosphere: AtmospherePropsUBO;

const PI: f32 = 3.141592653589793;
const PRIMARY_STEPS: i32 = 5;
const LIGHT_STEPS: i32 = 3;
const EARTH_RADIUS: f32 = 6371000.0;
const ATMOSPHERE_RADIUS: f32 = 6471000.0;

fn ray_sphere_intersection(origin: vec3<f32>, direction: vec3<f32>, radius: f32) -> vec2<f32> {
    let a = dot(direction, direction);
    let b = 2.0 * dot(direction, origin);
    let c = dot(origin, origin) - radius * radius;
    let discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return vec2<f32>(1e5, -1e5);
    }
    let root = sqrt(discriminant);
    return vec2<f32>((-b - root) / (2.0 * a), (-b + root) / (2.0 * a));
}

fn scatter(direction: vec3<f32>, origin: vec3<f32>, sun: vec3<f32>) -> vec4<f32> {
    let rayleigh_coefficient = vec3<f32>(5.5e-6, 13.0e-6, 22.4e-6);
    let mie_coefficient = 21e-6;
    let rayleigh_height = 8e3;
    let mie_height = 1.2e3;
    let mie_direction = 0.758;
    let ray = normalize(direction);
    let sun_direction = normalize(sun);

    var bounds = ray_sphere_intersection(origin, ray, ATMOSPHERE_RADIUS);
    if (bounds.x > bounds.y) {
        return vec4<f32>(0.0, 0.0, 0.0, 1.0);
    }
    bounds.x = max(bounds.x, 0.0);
    let ground = ray_sphere_intersection(origin, ray, EARTH_RADIUS);
    if (ground.x <= ground.y && ground.x > 0.0) {
        bounds.y = min(bounds.y, ground.x);
    }

    let primary_step = (bounds.y - bounds.x) / f32(PRIMARY_STEPS);
    var primary_time = bounds.x + primary_step * 0.5;
    var primary_rayleigh_depth = 0.0;
    var primary_mie_depth = 0.0;
    var total_rayleigh = vec3<f32>(0.0);
    var total_mie = vec3<f32>(0.0);
    let mu = dot(ray, sun_direction);
    let mu_squared = mu * mu;
    let g_squared = mie_direction * mie_direction;
    let rayleigh_phase = 3.0 / (16.0 * PI) * (1.0 + mu_squared);
    let mie_phase = 3.0 / (8.0 * PI) * ((1.0 - g_squared) * (mu_squared + 1.0)) /
        (pow(1.0 + g_squared - 2.0 * mu * mie_direction, 1.5) * (2.0 + g_squared));

    for (var i: i32 = 0; i < PRIMARY_STEPS; i++) {
        let sample_position = origin + ray * primary_time;
        let sample_height = length(sample_position) - EARTH_RADIUS;
        let rayleigh_step = exp(-sample_height / rayleigh_height) * primary_step;
        let mie_step = exp(-sample_height / mie_height) * primary_step;
        primary_rayleigh_depth += rayleigh_step;
        primary_mie_depth += mie_step;
        let light_step = ray_sphere_intersection(sample_position, sun_direction, ATMOSPHERE_RADIUS).y /
            f32(LIGHT_STEPS);
        var light_time = light_step * 0.5;
        var light_rayleigh_depth = 0.0;
        var light_mie_depth = 0.0;
        for (var j: i32 = 0; j < LIGHT_STEPS; j++) {
            let light_position = sample_position + sun_direction * light_time;
            let light_height = length(light_position) - EARTH_RADIUS;
            light_rayleigh_depth += exp(-light_height / rayleigh_height) * light_step;
            light_mie_depth += exp(-light_height / mie_height) * light_step;
            light_time += light_step;
        }
        let attenuation = exp(-(mie_coefficient * (primary_mie_depth + light_mie_depth) +
            rayleigh_coefficient * (primary_rayleigh_depth + light_rayleigh_depth)));
        total_rayleigh += rayleigh_step * attenuation;
        total_mie += mie_step * attenuation;
        primary_time += primary_step;
    }

    let opacity = exp(-(length(rayleigh_coefficient) * length(total_rayleigh) +
        mie_coefficient * length(total_mie)));
    let color = 22.0 * (rayleigh_phase * rayleigh_coefficient * total_rayleigh +
        mie_phase * mie_coefficient * total_mie);
    return vec4<f32>(color, opacity);
}

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let camera = atmosphere.camera_position.xyz * EARTH_RADIUS;
    var color = scatter(in.view_direction, camera, atmosphere.sun_position.xyz);
    color = vec4<f32>(1.0 - exp(-color.rgb), color.a);
    color = pow(color, vec4<f32>(1.0 / 2.2));
    return vec4<f32>(color.rgb, 1.0 - color.a) * atmosphere.sun_position.w;
}
)";
};

} // namespace shaders
} // namespace mln
