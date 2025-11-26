#pragma once

#include <mbgl/shaders/hillshade_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto hillshadeShaderPrelude = R"(

enum {
    idHillshadeDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idHillshadeTilePropsUBO = idDrawableReservedFragmentOnlyUBO,
    idHillshadeEvaluatedPropsUBO = drawableReservedUBOCount,
    hillshadeUBOCount
};

// Redefined Metal structs to match the memory layout from hillshade_layer_ubo.hpp
struct alignas(16) HillshadeDrawableUBO {
    /* 0 */ float4x4 matrix;
    /* 64 */
};

struct alignas(16) HillshadeTilePropsUBO {
    /* 0 */ float2 latrange;
    /* 8 */ float exaggeration;  // NEW: vertical exaggeration factor
    /* 12 */ int method;         // NEW: hillshade method (Standard, Combined, Igor, Multidirectional, Basic)
    /* 16 */ int num_lights;     // NEW: number of light sources (1-4)
    /* 20 */ float pad0;
    /* 24 */ float pad1;
    /* 28 */ float pad2;
    /* 32 */
};

struct alignas(16) HillshadeEvaluatedPropsUBO {
    /* 0 */ float4 accent;
    /* 16 */ float altitudes[4];   // NEW: altitude values in radians (up to 4 lights)
    /* 32 */ float azimuths[4];    // NEW: azimuth values in radians (up to 4 lights)
    /* 48 */ float4 shadows[4];     // NEW: shadow colors (up to 4 lights)
    /* 112 */ float4 highlights[4];  // NEW: highlight colors (up to 4 lights)
    /* 176 */
};

)";

template <>
struct ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "HillshadeShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = hillshadeShaderPrelude;
    static constexpr auto source = R"(

#define PI 3.141592653589793
#define STANDARD 0
#define COMBINED 1
#define IGOR 2
#define MULTIDIRECTIONAL 3
#define BASIC 4

struct VertexStage {
    short2 pos [[attribute(hillshadeUBOCount + 0)]];
    short2 texture_pos [[attribute(hillshadeUBOCount + 1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const HillshadeDrawableUBO* drawableVector [[buffer(idHillshadeDrawableUBO)]]) {

    device const HillshadeDrawableUBO& drawable = drawableVector[uboIndex];

    const float4 position = drawable.matrix * float4(float2(vertx.pos), 0, 1);
    float2 pos = float2(vertx.texture_pos) / 8192.0;
    pos.y = 1.0 - pos.y; // Flip Y for Metal texture coordinates

    return {
        .position    = position,
        .pos         = pos,
    };
}

// Helper function to calculate aspect (normalized direction of slope)
float get_aspect(float2 deriv) {
    return deriv.x != 0.0 ? atan2(deriv.y, -deriv.x) : PI / 2.0 * (deriv.y > 0.0 ? 1.0 : -1.0);
}

// MapLibre's legacy hillshade algorithm (Method 0: STANDARD)
void standard_hillshade(float2 deriv, device const HillshadeEvaluatedPropsUBO& props, thread half4& fragColor) {
    float azimuth = props.azimuths[0] + PI;
    float slope = atan(0.625 * length(deriv));
    
    // We scale the slope exponentially based on intensity, using a calculation similar to
    // the exponential interpolation function in the style spec:
    // https://github.com/mapbox/mapbox-gl-js/blob/master/src/style-spec/expression/definitions/interpolate.js#L217-L228
    // so that higher intensity values create more opaque hillshading.
    float intensity = sin(props.altitudes[0]);
    float base = 1.875 - intensity * 1.75;
    float maxValue = 0.5 * PI;
    float scaledSlope = intensity != 0.5 ? ((pow(base, slope) - 1.0) / (pow(base, maxValue) - 1.0)) * maxValue : slope;

    // The accent color is calculated with the cosine of the slope while the shade color is calculated with the sine
    // so that the accent color's rate of change eases in while the shade color's eases out.
    float accent = cos(scaledSlope);
    
    // We multiply both the accent and shade color by a clamped intensity value
    // so that intensities >= 0.5 do not additionally affect the color values
    // while intensity values < 0.5 make the overall color more transparent.
    float intensity_scale = clamp(intensity * 2.0, 0.0, 1.0);
    float4 accent_color = (1.0 - accent) * props.accent * intensity_scale;
    
    float shade = abs(glMod((get_aspect(deriv) + azimuth) / PI + 0.5, 2.0) - 1.0);
    float4 shade_color = mix(props.shadows[0], props.highlights[0], shade) * sin(scaledSlope) * intensity_scale;
    
    fragColor = half4(accent_color * (1.0 - shade_color.a) + shade_color);
}

// Mapbox-style hillshade (Method 4: BASIC)
void basic_hillshade(float2 deriv, device const HillshadeEvaluatedPropsUBO& props, thread half4& fragColor) {
    float azimuth = props.azimuths[0] + PI;
    float slope = atan(length(deriv));
    
    // Uses the same exponential scaling logic as STANDARD hillshade
    float intensity = sin(props.altitudes[0]);
    float base = 1.875 - intensity * 1.75;
    float maxValue = 0.5 * PI;
    float scaledSlope = intensity != 0.5 ? ((pow(base, slope) - 1.0) / (pow(base, maxValue) - 1.0)) * maxValue : slope;

    float accent = cos(scaledSlope);
    float intensity_scale = clamp(intensity * 2.0, 0.0, 1.0);
    float4 accent_color = (1.0 - accent) * props.accent * intensity_scale;
    
    float shade = abs(glMod((get_aspect(deriv) + azimuth) / PI + 0.5, 2.0) - 1.0);
    float4 shade_color = mix(props.shadows[0], props.highlights[0], shade) * sin(scaledSlope) * intensity_scale;
    
    fragColor = half4(accent_color * (1.0 - shade_color.a) + shade_color);
}

// Multi-light hillshade algorithm (Method 1: COMBINED)
void combined_hillshade(float2 deriv, device const HillshadeTilePropsUBO& tileProps, device const HillshadeEvaluatedPropsUBO& props, thread half4& fragColor) {
    float2 deriv_scaled = deriv * tileProps.exaggeration * 2.0;
    
    float4 total_shade_color = float4(0.0);
    float total_intensity_scale = 0.0;
    
    int num_lights = min(tileProps.num_lights, 4);

    // Sum the contribution of each light source
    for (int i = 0; i < num_lights; i++) {
        float altitude = props.altitudes[i];
        float azimuth = props.azimuths[i] + PI;
        float intensity = sin(altitude);
        
        float cang = abs(cos(altitude)); // Cosine of altitude
        float shade = cang * atan(length(deriv_scaled)) * 4.0 / PI / PI;
        float highlight = (PI / 2.0 - cang) * atan(length(deriv_scaled)) * 4.0 / PI / PI;

        float4 shade_color = props.shadows[i] * shade;
        float4 highlight_color = props.highlights[i] * highlight;
        
        float intensity_scale = clamp(intensity * 2.0, 0.0, 1.0);

        total_shade_color += (shade_color + highlight_color) * intensity_scale;
        total_intensity_scale += intensity_scale;
    }
    
    if (total_intensity_scale > 0.0) {
        fragColor = half4(total_shade_color / total_intensity_scale);
    } else {
        fragColor = half4(0.0);
    }
}

// Igor's hillshade algorithm (Method 2: IGOR)
void igor_hillshade(float2 deriv, device const HillshadeTilePropsUBO& tileProps, device const HillshadeEvaluatedPropsUBO& props, thread half4& fragColor) {
    // A unique hillshade method that calculates shadow/highlight strength based on both slope and aspect.
    // Only supports one light source (index 0).
    float2 deriv_scaled = deriv * tileProps.exaggeration * 2.0;
    float aspect = get_aspect(deriv_scaled);
    float azimuth = props.azimuths[0] + PI;
    float slope_strength = atan(length(deriv_scaled)) * 2.0 / PI;
    float aspect_strength = 1.0 - abs(glMod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);
    float shadow_strength = slope_strength * aspect_strength;
    float highlight_strength = slope_strength * (1.0 - aspect_strength);
    fragColor = half4(props.shadows[0] * shadow_strength + props.highlights[0] * highlight_strength);
}

// Multidirectional hillshade algorithm (Method 3: MULTIDIRECTIONAL)
void multidirectional_hillshade(float2 deriv, device const HillshadeTilePropsUBO& tileProps, device const HillshadeEvaluatedPropsUBO& props, thread half4& fragColor) {
    // Uses the full vector dot product to calculate slope strength, allowing light sources to be treated as vectors.
    float2 deriv_scaled = deriv * tileProps.exaggeration * 2.0;
    
    float4 total_color = float4(0.0);
    
    int num_lights = min(tileProps.num_lights, 4);

    for (int i = 0; i < num_lights; i++) {
        float altitude = props.altitudes[i];
        float azimuth = props.azimuths[i] + PI;
        
        float intensity = sin(altitude);
        
        float cang = cos(altitude);
        
        // Calculate the light vector and slope vector dot product
        float slope_strength = dot(normalize(float3(-deriv_scaled.x, -deriv_scaled.y, 1.0)), 
                                   float3(sin(azimuth) * cang, cos(azimuth) * cang, sin(altitude)));
        
        float4 light_color = slope_strength > 0.0 
                            ? props.highlights[i] * slope_strength 
                            : props.shadows[i] * abs(slope_strength);
        
        total_color += light_color;
    }
    
    fragColor = half4(total_color / float(num_lights));
}


half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const HillshadeTilePropsUBO* tilePropsVector [[buffer(idHillshadeTilePropsUBO)]],
                            device const HillshadeEvaluatedPropsUBO& props [[buffer(idHillshadeEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const HillshadeTilePropsUBO& tileProps = tilePropsVector[uboIndex];
    thread half4 fragColor;

    float4 pixel = image.sample(image_sampler, in.pos);

    // The derivative is passed via the red and green channels from the prepare stage, 
    // scaled to [-1, 1] range. It needs to be converted back to world-space slope.
    // Scale back from [0, 1] range in texture to [-4, 4] range used by the GLSL functions.
    float2 deriv = ((pixel.rg * 8.0) - 4.0);
    
    // Account for mercator projection distortion (same as GLSL)
    // The scale factor is based on the cosine of the pixel's approximate latitude.
    float scaleFactor = cos(radians((tileProps.latrange.x - tileProps.latrange.y) * (1.0 - in.pos.y) + tileProps.latrange.y));
    deriv = deriv / scaleFactor;


    // Use the switch logic via if/else if chain to select the hillshade method
    if (tileProps.method == STANDARD) {
        standard_hillshade(deriv, props, fragColor);
    } else if (tileProps.method == COMBINED) {
        combined_hillshade(deriv, tileProps, props, fragColor);
    } else if (tileProps.method == IGOR) {
        igor_hillshade(deriv, tileProps, props, fragColor);
    } else if (tileProps.method == MULTIDIRECTIONAL) {
        multidirectional_hillshade(deriv, tileProps, props, fragColor);
    } else {
        // Default to BASIC if method is unknown or BASIC
        basic_hillshade(deriv, props, fragColor);
    }

    return fragColor;
}
)";
};

} // namespace shaders
} // namespace mbgl
