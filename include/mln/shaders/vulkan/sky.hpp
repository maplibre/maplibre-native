#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/vulkan/shader_program.hpp>

namespace mln {
namespace shaders {

constexpr auto skyShaderPrelude = R"(

#define idSkyPropsUBO layerUBOStartId
#define idAtmospherePropsUBO layerUBOStartId

)";

template <>
struct ShaderSource<BuiltIn::SkyShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "SkyShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    static constexpr auto prelude = skyShaderPrelude;

    static constexpr auto vertex = R"(
layout(location = 0) in ivec2 in_position;
layout(location = 0) out vec2 frag_pos;
layout(location = 1) out vec3 frag_view_direction;

layout(set = LAYER_SET_INDEX, binding = idSkyPropsUBO) uniform SkyPropsUBO {
    mat4 inv_projection;
    vec4 sky_color;
    vec4 horizon_color;
    vec4 globe;
    vec2 horizon;
    vec2 horizon_normal;
    vec2 viewport_size;
    float sky_horizon_blend;
    float sky_blend;
    float camera_to_center_distance;
    float pixel_ratio;
    float atmosphere_blend;
    float atmosphere_height;
} sky;

void main() {
    frag_pos = vec2(in_position);
    frag_view_direction = (sky.inv_projection * vec4(frag_pos, 0.0, 1.0)).xyz;
    gl_Position = vec4(frag_pos, 1.0, 1.0);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) in vec2 frag_pos;
layout(location = 1) in vec3 frag_view_direction;
layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idSkyPropsUBO) uniform SkyPropsUBO {
    mat4 inv_projection;
    vec4 sky_color;
    vec4 horizon_color;
    vec4 globe;
    vec2 horizon;
    vec2 horizon_normal;
    vec2 viewport_size;
    float sky_horizon_blend;
    float sky_blend;
    float camera_to_center_distance;
    float pixel_ratio;
    float atmosphere_blend;
    float atmosphere_height;
} sky;

void main() {
    vec2 pixel = (frag_pos * 0.5 + 0.5) * sky.viewport_size;
    float blend = dot(pixel - sky.horizon, sky.horizon_normal);
    float opacity = 1.0;
    if (sky.sky_blend > 0.0) {
        vec3 ray = normalize(frag_view_direction);
        vec3 globe_position = sky.globe.xyz;
        float globe_radius = sky.globe.w;
        float globe_distance = length(globe_position);
        float angle_to_globe_center = acos(clamp(dot(ray, globe_position) / globe_distance, -1.0, 1.0));
        float horizon_angle = asin(min(globe_radius / globe_distance, 1.0));
        blend = mix(blend, ((angle_to_globe_center - horizon_angle) * sky.camera_to_center_distance + 1.0) * sky.pixel_ratio, sky.sky_blend);
        float closest_approach = max(dot(ray, globe_position), 0.0);
        float lowest_altitude = sqrt(max(globe_distance * globe_distance - closest_approach * closest_approach, 0.0)) - globe_radius;
        float atmosphere_height = sky.atmosphere_height;
        float inside_atmosphere = 1.0 - smoothstep(atmosphere_height, 10.0 * atmosphere_height, globe_distance - globe_radius);
        float through_atmosphere = 1.0 - smoothstep(0.0, atmosphere_height, lowest_altitude);
        opacity = inside_atmosphere * through_atmosphere * (1.0 - sky.atmosphere_blend);
    }

    vec4 color = vec4(0.0);
    if (blend > 0.0) {
        if (blend < sky.sky_horizon_blend) {
            float t = 1.0 - blend / sky.sky_horizon_blend;
            color = mix(sky.sky_color, sky.horizon_color, t * t);
        } else {
            color = sky.sky_color;
        }
    }
    out_color = color * mix(1.0, opacity, sky.sky_blend);
}
)";
};

template <>
struct ShaderSource<BuiltIn::AtmosphereShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "AtmosphereShader";
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;
    static constexpr auto prelude = skyShaderPrelude;

    static constexpr auto vertex = R"(
layout(location = 0) in ivec2 in_position;
layout(location = 0) out vec3 frag_view_direction;

layout(set = LAYER_SET_INDEX, binding = idAtmospherePropsUBO) uniform AtmospherePropsUBO {
    mat4 inv_projection;
    vec4 globe;
    vec4 sun_position;
} atmosphere;

void main() {
    vec2 pos = vec2(in_position);
    frag_view_direction = (atmosphere.inv_projection * vec4(pos, 0.0, 1.0)).xyz;
    // Vulkan uses a zero-to-one NDC depth range, so 0.5 matches WebGL's
    // framebuffer depth for clip-space z=0.
    gl_Position = vec4(pos, 0.5, 1.0);
    applySurfaceTransform();
}
)";

    static constexpr auto fragment = R"(
layout(location = 0) in vec3 frag_view_direction;
layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idAtmospherePropsUBO) uniform AtmospherePropsUBO {
    mat4 inv_projection;
    vec4 globe;
    vec4 sun_position;
} atmosphere;

const float PI = 3.141592653589793;
const int PRIMARY_STEPS = 5;
const int LIGHT_STEPS = 3;
const float EARTH_RADIUS = 6371000.0;
const float ATMOSPHERE_RADIUS = 6471000.0;

vec2 ray_sphere_intersection(vec3 origin, vec3 direction, float radius) {
    float a = dot(direction, direction);
    float b = 2.0 * dot(direction, origin);
    float c = dot(origin, origin) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return vec2(1e5, -1e5);
    float root = sqrt(discriminant);
    return vec2((-b - root) / (2.0 * a), (-b + root) / (2.0 * a));
}

vec4 scatter(vec3 direction, vec3 origin, vec3 sun) {
    const vec3 rayleigh_coefficient = vec3(5.5e-6, 13.0e-6, 22.4e-6);
    const float mie_coefficient = 21e-6;
    const float rayleigh_height = 8e3;
    const float mie_height = 1.2e3;
    const float mie_direction = 0.758;
    direction = normalize(direction);
    sun = normalize(sun);

    vec2 bounds = ray_sphere_intersection(origin, direction, ATMOSPHERE_RADIUS);
    if (bounds.x > bounds.y) return vec4(0.0, 0.0, 0.0, 1.0);
    bounds.x = max(bounds.x, 0.0);
    vec2 ground = ray_sphere_intersection(origin, direction, EARTH_RADIUS);
    if (ground.x <= ground.y && ground.x > 0.0) bounds.y = min(bounds.y, ground.x);

    float primary_step = (bounds.y - bounds.x) / float(PRIMARY_STEPS);
    float primary_time = bounds.x + primary_step * 0.5;
    float primary_rayleigh_depth = 0.0;
    float primary_mie_depth = 0.0;
    vec3 total_rayleigh = vec3(0.0);
    vec3 total_mie = vec3(0.0);
    float mu = dot(direction, sun);
    float mu_squared = mu * mu;
    float g_squared = mie_direction * mie_direction;
    float rayleigh_phase = 3.0 / (16.0 * PI) * (1.0 + mu_squared);
    float mie_phase = 3.0 / (8.0 * PI) * ((1.0 - g_squared) * (mu_squared + 1.0)) /
        (pow(1.0 + g_squared - 2.0 * mu * mie_direction, 1.5) * (2.0 + g_squared));

    for (int i = 0; i < PRIMARY_STEPS; ++i) {
        vec3 sample_position = origin + direction * primary_time;
        float sample_height = length(sample_position) - EARTH_RADIUS;
        float rayleigh_step = exp(-sample_height / rayleigh_height) * primary_step;
        float mie_step = exp(-sample_height / mie_height) * primary_step;
        primary_rayleigh_depth += rayleigh_step;
        primary_mie_depth += mie_step;
        float light_step = ray_sphere_intersection(sample_position, sun, ATMOSPHERE_RADIUS).y /
            float(LIGHT_STEPS);
        float light_time = light_step * 0.5;
        float light_rayleigh_depth = 0.0;
        float light_mie_depth = 0.0;
        for (int j = 0; j < LIGHT_STEPS; ++j) {
            vec3 light_position = sample_position + sun * light_time;
            float light_height = length(light_position) - EARTH_RADIUS;
            light_rayleigh_depth += exp(-light_height / rayleigh_height) * light_step;
            light_mie_depth += exp(-light_height / mie_height) * light_step;
            light_time += light_step;
        }
        vec3 attenuation = exp(-(mie_coefficient * (primary_mie_depth + light_mie_depth) +
            rayleigh_coefficient * (primary_rayleigh_depth + light_rayleigh_depth)));
        total_rayleigh += rayleigh_step * attenuation;
        total_mie += mie_step * attenuation;
        primary_time += primary_step;
    }

    float opacity = exp(-(length(rayleigh_coefficient) * length(total_rayleigh) +
        mie_coefficient * length(total_mie)));
    vec3 color = 22.0 * (rayleigh_phase * rayleigh_coefficient * total_rayleigh +
        mie_phase * mie_coefficient * total_mie);
    return vec4(color, opacity);
}

void main() {
    vec3 camera = -atmosphere.globe.xyz * EARTH_RADIUS / atmosphere.globe.w;
    vec4 color = scatter(frag_view_direction, camera, atmosphere.sun_position.xyz);
    color.rgb = 1.0 - exp(-color.rgb);
    color = pow(color, vec4(1.0 / 2.2));
    out_color = vec4(color.rgb, 1.0 - color.a) * atmosphere.sun_position.w;
}
)";
};

} // namespace shaders
} // namespace mln
