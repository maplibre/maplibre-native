#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp> // Include the UBO definition

namespace mbgl {
namespace shaders {

constexpr auto hillshadeShaderPrelude = R"(

#define idHillshadeDrawableUBO          idDrawableReservedVertexOnlyUBO
#define idHillshadeTilePropsUBO         idDrawableReservedFragmentOnlyUBO
#define idHillshadeEvaluatedPropsUBO    layerUBOStartId // Next available UBO index

)";

template <>
struct ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "HillshadeShader";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = hillshadeShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec2 in_texture_position;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

// Matches HillshadeDrawableUBO
struct HillshadeDrawableUBO {
    mat4 matrix;
};

layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = idHillshadeDrawableUBO) readonly buffer HillshadeDrawableUBOVector {
    HillshadeDrawableUBO drawable_ubo[];
} drawableVector;

layout(location = 0) out vec2 frag_position;

void main() {
    const HillshadeDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

    gl_Position = drawable.matrix * vec4(in_position, 0, 1);
    frag_position = vec2(in_texture_position) / 8192.0;
}
)";
    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position;

layout(set = TEXTURE_SET_INDEX, binding = 0) uniform sampler2D u_image;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

#define PI 3.141592653589793
#define STANDARD 0
#define COMBINED 1
#define IGOR 2
#define MULTIDIRECTIONAL 3
#define BASIC 4

// Matches HillshadeTilePropsUBO
struct HillshadeTilePropsUBO {
    vec2 latrange;
    float exaggeration;
    int method;
    int num_lights;
    float pad0;
    float pad1;
    float pad2;
};

// Matches HillshadeEvaluatedPropsUBO
struct HillshadeEvaluatedPropsUBO {
    vec4 accent;
    vec4 altitudes;     // altitudes[4] -> vec4
    vec4 azimuths;      // azimuths[4] -> vec4
    vec4 shadows[4];
    vec4 highlights[4];
};

layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = idHillshadeTilePropsUBO) readonly buffer HillshadeTilePropsUBOVector {
    HillshadeTilePropsUBO tileProps_ubo[];
} tilePropsVector;

layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = idHillshadeEvaluatedPropsUBO) readonly buffer HillshadeEvaluatedPropsUBOVector {
    HillshadeEvaluatedPropsUBO props_ubo[];
} propsVector;


float get_aspect(vec2 deriv) {
    return deriv.x != 0.0 ?
    atan(deriv.y, -deriv.x) : PI / 2.0 * (deriv.y > 0.0 ? 1.0 : -1.0);
}

// MapLibre's legacy hillshade algorithm (Method 0: STANDARD)
void standard_hillshade(vec2 deriv, const HillshadeEvaluatedPropsUBO props) {
    float intensity = sin(props.altitudes.x); // Use intensity from first light
    float azimuth = props.azimuths.x + PI;
    float slope = atan(0.625 * length(deriv));
    float aspect = get_aspect(deriv);
    
    // We scale the slope exponentially based on intensity...
    float base = 1.875 - intensity * 1.75;
    float maxValue = 0.5 * PI;
    float scaledSlope = intensity != 0.5 ? ((pow(base, slope) - 1.0) / (pow(base, maxValue) - 1.0)) * maxValue : slope;

    float accent = cos(scaledSlope);
    float intensity_scale = clamp(intensity * 2.0, 0.0, 1.0);
    vec4 accent_color = (1.0 - accent) * props.accent * intensity_scale;
    
    float shade = abs(mod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);
    vec4 shade_color = mix(props.shadows[0], props.highlights[0], shade) * sin(scaledSlope) * intensity_scale;
    
    fragColor = accent_color * (1.0 - shade_color.a) + shade_color;
}

// Mapbox-style hillshade (Method 4: BASIC)
void basic_hillshade(vec2 deriv, const HillshadeTilePropsUBO tileProps, const HillshadeEvaluatedPropsUBO props) {
    deriv = deriv * tileProps.exaggeration * 2.0;
    float azimuth = props.azimuths.x + PI;
    float cos_az = cos(azimuth);
    float sin_az = sin(azimuth);
    float cos_alt = cos(props.altitudes.x);
    float sin_alt = sin(props.altitudes.x);
    
    // Calculate the cosine of the angle between the light vector and the surface normal
    float cang = (sin_alt - (deriv.y * cos_az * cos_alt - deriv.x * sin_az * cos_alt)) / sqrt(1.0 + dot(deriv, deriv));
    float shade = clamp(cang, 0.0, 1.0);
    
    if (shade > 0.5) {
        fragColor = props.highlights[0] * (2.0 * shade - 1.0);
    } else {
        fragColor = props.shadows[0] * (1.0 - 2.0 * shade);
    }
}

// Multi-light hillshade algorithm (Method 1: COMBINED)
void combined_hillshade(vec2 deriv, const HillshadeTilePropsUBO tileProps, const HillshadeEvaluatedPropsUBO props) {
    deriv = deriv * tileProps.exaggeration * 2.0;
    
    vec4 total_shade_color = vec4(0.0);
    float total_intensity_scale = 0.0;
    
    // Access altitude/azimuth from vec4 fields using array-like logic
    // We need to use conditional access since arrays of floats are mapped to vec4
    float altitudes[4] = float[4](props.altitudes.x, props.altitudes.y, props.altitudes.z, props.altitudes.w);
    float azimuths[4] = float[4](props.azimuths.x, props.azimuths.y, props.azimuths.z, props.azimuths.w);

    int num_lights = min(tileProps.num_lights, 4);

    // Sum the contribution of each light source
    for (int i = 0; i < num_lights; i++) {
        float altitude = altitudes[i];
        float azimuth = azimuths[i] + PI;
        float intensity = sin(altitude);
        
        float cos_az = cos(azimuth);
        float sin_az = sin(azimuth);
        float cos_alt = cos(altitude);
        float sin_alt = sin(altitude);

        // Calculate the angle between the light vector and the surface normal
        float cang_angle = acos((sin_alt - (deriv.y * cos_az * cos_alt - deriv.x * sin_az * cos_alt)) /
                                sqrt(1.0 + dot(deriv, deriv)));
        
        cang_angle = clamp(cang_angle, 0.0, PI / 2.0); // Clamp angle to 90 degrees (half hemisphere)
        
        // The "shade" and "highlight" components are calculated from cang_angle (angle) and the magnitude of the slope
        float shade = cang_angle * atan(length(deriv)) * 4.0 / PI / PI;
        float highlight = (PI / 2.0 - cang_angle) * atan(length(deriv)) * 4.0 / PI / PI;

        vec4 shade_color = props.shadows[i] * shade;
        vec4 highlight_color = props.highlights[i] * highlight;
        
        float intensity_scale = clamp(intensity * 2.0, 0.0, 1.0);

        total_shade_color += (shade_color + highlight_color) * intensity_scale;
        total_intensity_scale += intensity_scale;
    }
    
    if (total_intensity_scale > 0.0) {
        fragColor = total_shade_color / total_intensity_scale;
    } else {
        fragColor = vec4(0.0);
    }
}

// Igor's hillshade algorithm (Method 2: IGOR)
void igor_hillshade(vec2 deriv, const HillshadeTilePropsUBO tileProps, const HillshadeEvaluatedPropsUBO props) {
    // A unique hillshade method that calculates shadow/highlight strength based on both slope and aspect.
    // Only supports one light source (index 0).
    deriv = deriv * tileProps.exaggeration * 2.0;
    float aspect = get_aspect(deriv);
    float azimuth = props.azimuths.x + PI;
    float slope_strength = atan(length(deriv)) * 2.0 / PI;
    float aspect_strength = 1.0 - abs(mod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);
    float shadow_strength = slope_strength * aspect_strength;
    float highlight_strength = slope_strength * (1.0 - aspect_strength);
    fragColor = props.shadows[0] * shadow_strength + props.highlights[0] * highlight_strength;
}

// Multidirectional hillshade algorithm (Method 3: MULTIDIRECTIONAL)
void multidirectional_hillshade(vec2 deriv, const HillshadeTilePropsUBO tileProps, const HillshadeEvaluatedPropsUBO props) {
    // Uses the full vector dot product to calculate slope strength, allowing light sources to be treated as vectors.
    deriv = deriv * tileProps.exaggeration * 2.0;
    
    vec4 total_color = vec4(0.0);
    
    float altitudes[4] = float[4](props.altitudes.x, props.altitudes.y, props.altitudes.z, props.altitudes.w);
    float azimuths[4] = float[4](props.azimuths.x, props.azimuths.y, props.azimuths.z, props.azimuths.w);

    int num_lights = min(tileProps.num_lights, 4);

    for (int i = 0; i < num_lights; i++) {
        float altitude = altitudes[i];
        float azimuth = azimuths[i]; // Azimuth is already in world-space direction for vector math
        
        float cos_alt = cos(altitude);
        float sin_alt = sin(altitude);
        
        // Light Vector (Lx, Ly, Lz)
        // Lx = cos(altitude) * sin(azimuth)
        // Ly = cos(altitude) * cos(azimuth)
        // Lz = sin(altitude)
        
        // Surface Normal Vector (Nx, Ny, Nz)
        // N = normalize(float3(-deriv.x, -deriv.y, 1.0))

        // Calculate the light vector and slope vector dot product (dot(N, L))
        float slope_strength = dot(normalize(vec3(-deriv.x, -deriv.y, 1.0)), 
                                   vec3(sin(azimuth) * cos_alt, cos(azimuth) * cos_alt, sin_alt));
        
        vec4 light_color = slope_strength > 0.0 
                            ? props.highlights[i] * slope_strength 
                            : props.shadows[i] * abs(slope_strength);
        
        total_color += light_color;
    }
    
    fragColor = total_color / float(num_lights);
}


void main() {
    const HillshadeTilePropsUBO tileProps = tilePropsVector.tileProps_ubo[constant.ubo_index];
    const HillshadeEvaluatedPropsUBO props = propsVector.props_ubo[constant.ubo_index];

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
    return;
#endif

    vec4 pixel = texture(u_image, frag_position);

    // Scale the derivative based on the mercator distortion at this latitude
    float scaleFactor = cos(radians((tileProps.latrange[0] - tileProps.latrange[1]) * (1.0 - frag_position.y) + tileProps.latrange[1]));
    
    // The derivative is scaled back from [0, 1] texture range to world-space slope
    vec2 deriv = ((pixel.rg * 8.0) - 4.0) / scaleFactor;
    
    // Dispatch to the selected hillshade method
    if (tileProps.method == COMBINED) {
        combined_hillshade(deriv, tileProps, props);
    } else if (tileProps.method == IGOR) {
        igor_hillshade(deriv, tileProps, props);
    } else if (tileProps.method == MULTIDIRECTIONAL) {
        multidirectional_hillshade(deriv, tileProps, props);
    } else if (tileProps.method == BASIC) {
        basic_hillshade(deriv, tileProps, props);
    } else {
        // Default to STANDARD
        standard_hillshade(deriv, props);
    }
}
)";
};

} // namespace shaders
} // namespace mbgl
