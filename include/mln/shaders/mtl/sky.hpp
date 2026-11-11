#pragma once

#include <mln/shaders/mtl/shader_program.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/shader_source.hpp>

namespace mln {
namespace shaders {

constexpr auto skyShaderPrelude = R"(

enum {
    idSkyPropsUBO = drawableReservedUBOCount,
    skyUBOCount
};

enum {
    idAtmospherePropsUBO = drawableReservedUBOCount,
    atmosphereUBOCount
};

struct alignas(16) SkyPropsUBO {
    float4x4 inv_projection;
    float4 sky_color;
    float4 horizon_color;
    float4 globe;
    float2 horizon;
    float2 horizon_normal;
    float2 viewport_size;
    float sky_horizon_blend;
    float sky_blend;
    float camera_to_center_distance;
    float pixel_ratio;
    float atmosphere_blend;
    float atmosphere_height;
};

struct alignas(16) AtmospherePropsUBO {
    float4x4 inv_projection;
    float4 globe;
    float4 sun_position;
};

)";

template <>
struct ShaderSource<BuiltIn::SkyShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "SkyShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = skyShaderPrelude;
    static constexpr auto source = R"(

struct SkyVertexStage {
    short2 position [[attribute(0)]];
};

struct SkyFragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
    float3 view_direction;
};

SkyFragmentStage vertex vertexMain(SkyVertexStage in [[stage_in]],
                                   device const SkyPropsUBO& sky [[buffer(idSkyPropsUBO)]]) {
    const float2 pos = float2(in.position);
    return {
        .position = float4(pos, 1.0, 1.0),
        .pos = pos,
        .view_direction = (sky.inv_projection * float4(pos, 0.0, 1.0)).xyz,
    };
}

half4 fragment fragmentMain(SkyFragmentStage in [[stage_in]],
                            device const SkyPropsUBO& sky [[buffer(idSkyPropsUBO)]]) {
    const float2 pixel = (in.pos * 0.5 + 0.5) * sky.viewport_size;
    float blend = dot(pixel - sky.horizon, sky.horizon_normal);
    float opacity = 1.0;
    if (sky.sky_blend > 0.0) {
        const float3 ray = normalize(in.view_direction);
        const float3 globePosition = sky.globe.xyz;
        const float globeRadius = sky.globe.w;
        const float globeDistance = length(globePosition);
        const float angleToGlobeCenter = acos(clamp(dot(ray, globePosition) / globeDistance, -1.0, 1.0));
        const float horizonAngle = asin(min(globeRadius / globeDistance, 1.0));
        blend = mix(blend, ((angleToGlobeCenter - horizonAngle) * sky.camera_to_center_distance + 1.0) * sky.pixel_ratio,
                    sky.sky_blend);
        const float closestApproach = max(dot(ray, globePosition), 0.0);
        const float lowestAltitude =
            sqrt(max(globeDistance * globeDistance - closestApproach * closestApproach, 0.0)) - globeRadius;
        const float atmosphereHeight = sky.atmosphere_height;
        const float insideAtmosphere =
            1.0 - smoothstep(atmosphereHeight, 10.0 * atmosphereHeight, globeDistance - globeRadius);
        const float throughAtmosphere = 1.0 - smoothstep(0.0, atmosphereHeight, lowestAltitude);
        opacity = insideAtmosphere * throughAtmosphere * (1.0 - sky.atmosphere_blend);
    }

    float4 color = float4(0.0);
    if (blend > 0.0) {
        if (blend < sky.sky_horizon_blend) {
            const float t = 1.0 - blend / sky.sky_horizon_blend;
            color = mix(sky.sky_color, sky.horizon_color, t * t);
        } else {
            color = sky.sky_color;
        }
    }

    return half4(color * mix(1.0, opacity, sky.sky_blend));
}
)";
};

template <>
struct ShaderSource<BuiltIn::AtmosphereShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "AtmosphereShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = skyShaderPrelude;
    static constexpr auto source = R"(

struct AtmosphereVertexStage {
    short2 position [[attribute(0)]];
};

struct AtmosphereFragmentStage {
    float4 position [[position, invariant]];
    float3 view_direction;
};

AtmosphereFragmentStage vertex vertexMain(
    AtmosphereVertexStage in [[stage_in]],
    device const AtmospherePropsUBO& atmosphere [[buffer(idAtmospherePropsUBO)]]) {
    const float2 pos = float2(in.position);
    return {
        // Metal uses a zero-to-one NDC depth range, so 0.5 matches WebGL's
        // framebuffer depth for clip-space z=0.
        .position = float4(pos, 0.5, 1.0),
        .view_direction = (atmosphere.inv_projection * float4(pos, 0.0, 1.0)).xyz,
    };
}

constant float AtmospherePI = 3.141592653589793;
constant int AtmospherePrimarySteps = 5;
constant int AtmosphereLightSteps = 3;
constant float EarthRadius = 6371000.0;
constant float AtmosphereRadius = 6471000.0;

float2 atmosphereRaySphereIntersection(float3 origin, float3 direction, float radius) {
    const float a = dot(direction, direction);
    const float b = 2.0 * dot(direction, origin);
    const float c = dot(origin, origin) - radius * radius;
    const float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return float2(1e5, -1e5);
    }
    const float root = sqrt(discriminant);
    return float2((-b - root) / (2.0 * a), (-b + root) / (2.0 * a));
}

float4 atmosphereScatter(float3 direction, float3 origin, float3 sun) {
    const float3 rayleighCoefficient = float3(5.5e-6, 13.0e-6, 22.4e-6);
    const float mieCoefficient = 21e-6;
    const float rayleighHeight = 8e3;
    const float mieHeight = 1.2e3;
    const float mieDirection = 0.758;

    direction = normalize(direction);
    sun = normalize(sun);

    float2 bounds = atmosphereRaySphereIntersection(origin, direction, AtmosphereRadius);
    if (bounds.x > bounds.y) {
        return float4(0.0, 0.0, 0.0, 1.0);
    }
    bounds.x = max(bounds.x, 0.0);

    const float2 ground = atmosphereRaySphereIntersection(origin, direction, EarthRadius);
    if (ground.x <= ground.y && ground.x > 0.0) {
        bounds.y = min(bounds.y, ground.x);
    }

    const float primaryStep = (bounds.y - bounds.x) / float(AtmospherePrimarySteps);
    float primaryTime = bounds.x + primaryStep * 0.5;
    float primaryRayleighDepth = 0.0;
    float primaryMieDepth = 0.0;
    float3 totalRayleigh = float3(0.0);
    float3 totalMie = float3(0.0);

    const float mu = dot(direction, sun);
    const float muSquared = mu * mu;
    const float gSquared = mieDirection * mieDirection;
    const float rayleighPhase = 3.0 / (16.0 * AtmospherePI) * (1.0 + muSquared);
    const float miePhase = 3.0 / (8.0 * AtmospherePI) * ((1.0 - gSquared) * (muSquared + 1.0)) /
        (pow(1.0 + gSquared - 2.0 * mu * mieDirection, 1.5) * (2.0 + gSquared));

    for (int i = 0; i < AtmospherePrimarySteps; ++i) {
        const float3 samplePosition = origin + direction * primaryTime;
        const float sampleHeight = length(samplePosition) - EarthRadius;
        const float rayleighStep = exp(-sampleHeight / rayleighHeight) * primaryStep;
        const float mieStep = exp(-sampleHeight / mieHeight) * primaryStep;
        primaryRayleighDepth += rayleighStep;
        primaryMieDepth += mieStep;

        const float lightStep = atmosphereRaySphereIntersection(samplePosition, sun, AtmosphereRadius).y /
            float(AtmosphereLightSteps);
        float lightTime = lightStep * 0.5;
        float lightRayleighDepth = 0.0;
        float lightMieDepth = 0.0;
        for (int j = 0; j < AtmosphereLightSteps; ++j) {
            const float3 lightPosition = samplePosition + sun * lightTime;
            const float lightHeight = length(lightPosition) - EarthRadius;
            lightRayleighDepth += exp(-lightHeight / rayleighHeight) * lightStep;
            lightMieDepth += exp(-lightHeight / mieHeight) * lightStep;
            lightTime += lightStep;
        }

        const float3 attenuation = exp(-(mieCoefficient * (primaryMieDepth + lightMieDepth) +
            rayleighCoefficient * (primaryRayleighDepth + lightRayleighDepth)));
        totalRayleigh += rayleighStep * attenuation;
        totalMie += mieStep * attenuation;
        primaryTime += primaryStep;
    }

    const float opacity = exp(-(length(rayleighCoefficient) * length(totalRayleigh) +
        mieCoefficient * length(totalMie)));
    const float3 color = 22.0 * (rayleighPhase * rayleighCoefficient * totalRayleigh +
        miePhase * mieCoefficient * totalMie);
    return float4(color, opacity);
}

half4 fragment fragmentMain(
    AtmosphereFragmentStage in [[stage_in]],
    device const AtmospherePropsUBO& atmosphere [[buffer(idAtmospherePropsUBO)]]) {
    const float3 camera = -atmosphere.globe.xyz * EarthRadius / atmosphere.globe.w;
    float4 color = atmosphereScatter(in.view_direction, camera, atmosphere.sun_position.xyz);
    color.rgb = 1.0 - exp(-color.rgb);
    color = pow(color, float4(1.0 / 2.2));
    return half4(float4(color.rgb, 1.0 - color.a) * atmosphere.sun_position.w);
}
)";
};

} // namespace shaders
} // namespace mln
