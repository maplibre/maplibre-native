// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "HillshadeShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;

layout (std140) uniform HillshadeDrawableUBO {
    highp mat4 u_matrix;
};

out vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    v_pos = a_texture_pos / 8192.0;
}
)";
    static constexpr const char* fragment = R"(in vec2 v_pos;
uniform sampler2D u_image;

layout(std140) uniform HillshadeTilePropsUBO {
    highp vec2 u_latrange;
    highp float u_exaggeration;
    highp int u_method;           // Hillshade method (0: STANDARD, 1: COMBINED, 2: IGOR, 3: MULTIDIRECTIONAL, 4: BASIC)
    highp int u_num_lights;       // Number of light sources (1-4)
    highp float u_pad0;
    highp float u_pad1;
    highp float u_pad2;
};
layout(std140) uniform HillshadeEvaluatedPropsUBO {
    highp vec4 u_accent;
    highp vec4 u_altitudes;       // Up to 4 altitude values (in radians)
    highp vec4 u_azimuths;        // Up to 4 azimuth values (in radians)
    highp vec4 u_shadows[4];      // Shadow colors (up to 4 lights)
    highp vec4 u_highlights[4];   // Highlight colors (up to 4 lights)
};

#define PI 3.141592653589793

#define STANDARD 0
#define COMBINED 1
#define IGOR 2
#define MULTIDIRECTIONAL 3
#define BASIC 4

float get_aspect(vec2 deriv) {
    return deriv.x != 0.0 ?
    atan(deriv.y, -deriv.x) : PI / 2.0 * (deriv.y > 0.0 ? 1.0 : -1.0);
}

// MapLibre's legacy hillshade algorithm (Method 0)
void standard_hillshade(vec2 deriv) {
    float azimuth = u_azimuths.x + PI;
    float slope = atan(0.625 * length(deriv));
    float aspect = get_aspect(deriv);

    // Note: This implementation uses u_exaggeration as intensity, though it typically should be derived from u_altitudes.x.
    float intensity = u_exaggeration;

    // Scale the slope exponentially based on intensity
    float base = 1.875 - intensity * 1.75;
    float maxValue = 0.5 * PI;
    float scaledSlope = intensity != 0.5 ? ((pow(base, slope) - 1.0) / (pow(base, maxValue) - 1.0)) * maxValue : slope;

    float accent = cos(scaledSlope);
    vec4 accent_color = (1.0 - accent) * u_accent * clamp(intensity * 2.0, 0.0, 1.0);

    // Calculate shadow/highlight based on aspect
    float shade = abs(mod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);
    vec4 shade_color = mix(u_shadows[0], u_highlights[0], shade) * sin(scaledSlope) * clamp(intensity * 2.0, 0.0, 1.0);

    fragColor = accent_color * (1.0 - shade_color.a) + shade_color;
}

// Basic directional hillshade (Method 4)
void basic_hillshade(vec2 deriv) {
    deriv = deriv * u_exaggeration * 2.0; // Exaggerate the slope derivative
    float azimuth = u_azimuths.x + PI;
    float cos_az = cos(azimuth);
    float sin_az = sin(azimuth);
    float cos_alt = cos(u_altitudes.x);
    float sin_alt = sin(u_altitudes.x);

    // Calculate the cosine of the angle between the light vector and the surface normal
    float cang = (sin_alt - (deriv.y * cos_az * cos_alt - deriv.x * sin_az * cos_alt)) / sqrt(1.0 + dot(deriv, deriv));
    float shade = clamp(cang, 0.0, 1.0); // cang is the hillshade intensity [0, 1]

    // Blend shadow and highlight based on intensity
    if (shade > 0.5) {
        fragColor = u_highlights[0] * (2.0 * shade - 1.0); // Highlight strength [0, 1]
    } else {
        fragColor = u_shadows[0] * (1.0 - 2.0 * shade);    // Shadow strength [0, 1]
    }
}

// Multidirectional hillshade (Method 3)
void multidirectional_hillshade(vec2 deriv) {
    deriv = deriv * u_exaggeration * 2.0; // Exaggerate the slope derivative
    fragColor = vec4(0, 0, 0, 0);

    // Iterate through all light sources (up to u_num_lights, max 4)
    for (int i = 0; i < u_num_lights; i++) {
        // Access altitude and azimuth from vec4 UBOs
        float altitude = (i == 0) ? u_altitudes.x : (i == 1) ? u_altitudes.y : (i == 2) ? u_altitudes.z : u_altitudes.w;
        float azimuth = (i == 0) ? u_azimuths.x : (i == 1) ? u_azimuths.y : (i == 2) ? u_azimuths.z : u_azimuths.w;

        float cos_alt = cos(altitude);
        float sin_alt = sin(altitude);
        // Negate cos/sin azimuth for correct light direction in the normal calculation
        float cos_az = -cos(azimuth);
        float sin_az = -sin(azimuth);

        // Calculate the cosine of the angle between the light vector and the surface normal
        float cang = (sin_alt - (deriv.y * cos_az * cos_alt - deriv.x * sin_az * cos_alt)) /
                     sqrt(1.0 + dot(deriv, deriv));
        float shade = clamp(cang, 0.0, 1.0); // cang is the hillshade intensity [0, 1]

        // Accumulate shadow/highlight contribution from each light
        if (shade > 0.5) {
            fragColor += u_highlights[i] * (2.0 * shade - 1.0) / float(u_num_lights);
        } else {
            fragColor += u_shadows[i] * (1.0 - 2.0 * shade) / float(u_num_lights);
        }
    }
}

// Combined shadow and highlight method (Method 1)
void combined_hillshade(vec2 deriv) {
    // Only supports one light source (index 0)
    deriv = deriv * u_exaggeration * 2.0;
    float azimuth = u_azimuths.x + PI;
    float cos_az = cos(azimuth);
    float sin_az = sin(azimuth);
    float cos_alt = cos(u_altitudes.x);
    float sin_alt = sin(u_altitudes.x);

    // Calculate the angle between the light vector and the surface normal
    float cang = acos((sin_alt - (deriv.y * cos_az * cos_alt - deriv.x * sin_az * cos_alt)) /
                      sqrt(1.0 + dot(deriv, deriv)));

    cang = clamp(cang, 0.0, PI / 2.0); // Clamp angle to 90 degrees (half hemisphere)

    // The "shade" and "highlight" components are calculated from cang (angle) and the magnitude of the slope
    float shade = cang * atan(length(deriv)) * 4.0 / PI / PI;
    float highlight = (PI / 2.0 - cang) * atan(length(deriv)) * 4.0 / PI / PI;

    fragColor = u_shadows[0] * shade + u_highlights[0] * highlight;
}

// Igor's shadow/highlight method (Method 2)
void igor_hillshade(vec2 deriv) {
    // Only supports one light source (index 0)
    deriv = deriv * u_exaggeration * 2.0;
    float aspect = get_aspect(deriv);
    float azimuth = u_azimuths.x + PI;

    // Slope strength is magnitude of slope vector, normalized to [0, 1]
    float slope_strength = atan(length(deriv)) * 2.0 / PI;

    // Aspect strength is difference between aspect and light azimuth, normalized to [0, 1]
    float aspect_strength = 1.0 - abs(mod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);

    float shadow_strength = slope_strength * aspect_strength;
    float highlight_strength = slope_strength * (1.0 - aspect_strength);

    fragColor = u_shadows[0] * shadow_strength + u_highlights[0] * highlight_strength;
}

void main() {
    vec4 pixel = texture(u_image, v_pos);

    // Scale the derivative based on the mercator distortion at this latitude (v_pos.y)
    float scaleFactor = cos(radians((u_latrange[0] - u_latrange[1]) * (1.0 - v_pos.y) + u_latrange[1]));

    // The derivative is scaled back from [0, 1] texture range to world-space slope
    // Texture range [0, 1] corresponds to slope range [-4, 4] (8.0 * 0.5 * deriv)
    vec2 deriv = ((pixel.rg * 8.0) - 4.0) / scaleFactor;

    // Dispatch to the selected hillshade method
    switch (u_method) {
        case BASIC:
            basic_hillshade(deriv);
            break;
        case COMBINED:
            combined_hillshade(deriv);
            break;
        case IGOR:
            igor_hillshade(deriv);
            break;
        case MULTIDIRECTIONAL:
            multidirectional_hillshade(deriv);
            break;
        case STANDARD:
        default:
            standard_hillshade(deriv);
            break;
    }

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl

