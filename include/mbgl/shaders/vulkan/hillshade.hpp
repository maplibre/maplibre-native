#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto hillshadeShaderPrelude = R"(

#define idHillshadeDrawableUBO          idDrawableReservedVertexOnlyUBO
#define idHillshadeTilePropsUBO         idDrawableReservedFragmentOnlyUBO
#define idHillshadeEvaluatedPropsUBO    layerUBOStartId

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

struct HillshadeDrawableUBO {
    mat4 matrix;
};

layout(std140, set = LAYER_SET_INDEX, binding = idHillshadeDrawableUBO) readonly buffer HillshadeDrawableUBOVector {
    HillshadeDrawableUBO drawable_ubo[];
} drawableVector;

layout(location = 0) out vec2 frag_position;

void main() {
    const HillshadeDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();

    frag_position = vec2(in_texture_position) / 8192.0;
    frag_position.y = 1.0 - frag_position.y;
}
)";

    static constexpr auto fragment = R"(

#define PI 3.141592653589793
#define STANDARD 0
#define COMBINED 1
#define IGOR 2
#define MULTIDIRECTIONAL 3
#define BASIC 4

layout(location = 0) in vec2 frag_position;
layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct HillshadeTilePropsUBO {
    vec2 latrange;
    float exaggeration;
    int method;
    int num_lights;
    float pad0;
    float pad1;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idHillshadeTilePropsUBO) readonly buffer HillshadeTilePropsUBOVector {
    HillshadeTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idHillshadeEvaluatedPropsUBO) uniform HillshadeEvaluatedPropsUBO {
    vec4 accent;
    vec4 altitudes;
    vec4 azimuths;
    vec4 shadows[4];
    vec4 highlights[4];
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;

float get_aspect(vec2 deriv) {
    return deriv.x != 0.0 ? atan(deriv.y, -deriv.x) : PI / 2.0 * (deriv.y > 0.0 ? 1.0 : -1.0);
}

// MapLibre's legacy hillshade algorithm (Method 0: STANDARD)
void standard_hillshade(vec2 deriv, const HillshadeTilePropsUBO tileProps) {
    float azimuth = props.azimuths.x + PI;
    float slope = atan(0.625 * length(deriv));
    float aspect = get_aspect(deriv);

    float intensity = tileProps.exaggeration;

    // Scale the slope exponentially based on intensity
    float base = 1.875 - intensity * 1.75;
    float maxValue = 0.5 * PI;
    float scaledSlope = intensity != 0.5 ? ((pow(base, slope) - 1.0) / (pow(base, maxValue) - 1.0)) * maxValue : slope;

    float accent = cos(scaledSlope);
    vec4 accent_color = (1.0 - accent) * props.accent * clamp(intensity * 2.0, 0.0, 1.0);

    float shade = abs(mod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);
    vec4 shade_color = mix(props.shadows[0], props.highlights[0], shade) * sin(scaledSlope) * clamp(intensity * 2.0, 0.0, 1.0);

    out_color = accent_color * (1.0 - shade_color.a) + shade_color;
}

// Basic directional hillshade (Method 4: BASIC)
void basic_hillshade(vec2 deriv, const HillshadeTilePropsUBO tileProps) {
    deriv = deriv * tileProps.exaggeration * 2.0;
    float azimuth = props.azimuths.x + PI;
    float cos_az = cos(azimuth);
    float sin_az = sin(azimuth);
    float cos_alt = cos(props.altitudes.x);
    float sin_alt = sin(props.altitudes.x);

    // Calculate the cosine of the angle between the light vector and the surface normal
    float cang = (sin_alt - (deriv.y * cos_az * cos_alt - deriv.x * sin_az * cos_alt)) / sqrt(1.0 + dot(deriv, deriv));
    float shade = clamp(cang, 0.0, 1.0);

    // Blend shadow and highlight based on intensity
    if (shade > 0.5) {
        out_color = props.highlights[0] * (2.0 * shade - 1.0);
    } else {
        out_color = props.shadows[0] * (1.0 - 2.0 * shade);
    }
}

// Multidirectional hillshade (Method 3: MULTIDIRECTIONAL)
void multidirectional_hillshade(vec2 deriv, const HillshadeTilePropsUBO tileProps) {
    deriv = deriv * tileProps.exaggeration * 2.0;
    vec4 total_color = vec4(0, 0, 0, 0);

    // Access altitude and azimuth from vec4 UBOs
    float altitudes[4] = float[4](props.altitudes.x, props.altitudes.y, props.altitudes.z, props.altitudes.w);
    float azimuths[4] = float[4](props.azimuths.x, props.azimuths.y, props.azimuths.z, props.azimuths.w);

    int num_lights = min(tileProps.num_lights, 4);

    // Iterate through all light sources
    for (int i = 0; i < num_lights; i++) {
        float altitude = altitudes[i];
        float azimuth = azimuths[i];

        float cos_alt = cos(altitude);
        float sin_alt = sin(altitude);
        // Negate cos/sin azimuth for correct light direction
        float cos_az = -cos(azimuth);
        float sin_az = -sin(azimuth);

        // Calculate the cosine of the angle between the light vector and the surface normal
        float cang = (sin_alt - (deriv.y * cos_az * cos_alt - deriv.x * sin_az * cos_alt)) / sqrt(1.0 + dot(deriv, deriv));
        float shade = clamp(cang, 0.0, 1.0);

        // Accumulate shadow/highlight contribution from each light
        if (shade > 0.5) {
            total_color += props.highlights[i] * (2.0 * shade - 1.0) / float(num_lights);
        } else {
            total_color += props.shadows[i] * (1.0 - 2.0 * shade) / float(num_lights);
        }
    }

    out_color = total_color;
}

// Combined shadow and highlight method (Method 1: COMBINED)
void combined_hillshade(vec2 deriv, const HillshadeTilePropsUBO tileProps) {
    // Only supports one light source (index 0)
    deriv = deriv * tileProps.exaggeration * 2.0;
    float azimuth = props.azimuths.x + PI;
    float cos_az = cos(azimuth);
    float sin_az = sin(azimuth);
    float cos_alt = cos(props.altitudes.x);
    float sin_alt = sin(props.altitudes.x);

    // Calculate the angle between the light vector and the surface normal
    float cang = acos((sin_alt - (deriv.y * cos_az * cos_alt - deriv.x * sin_az * cos_alt)) / sqrt(1.0 + dot(deriv, deriv)));

    cang = clamp(cang, 0.0, PI / 2.0);

    // Calculate shade and highlight components from angle and slope magnitude
    float shade = cang * atan(length(deriv)) * 4.0 / PI / PI;
    float highlight = (PI / 2.0 - cang) * atan(length(deriv)) * 4.0 / PI / PI;

    out_color = props.shadows[0] * shade + props.highlights[0] * highlight;
}

// Igor's shadow/highlight method (Method 2: IGOR)
void igor_hillshade(vec2 deriv, const HillshadeTilePropsUBO tileProps) {
    // Only supports one light source (index 0)
    deriv = deriv * tileProps.exaggeration * 2.0;
    float aspect = get_aspect(deriv);
    float azimuth = props.azimuths.x + PI;

    // Slope strength is magnitude of slope vector, normalized to [0, 1]
    float slope_strength = atan(length(deriv)) * 2.0 / PI;

    // Aspect strength is difference between aspect and light azimuth, normalized to [0, 1]
    float aspect_strength = 1.0 - abs(mod((aspect + azimuth) / PI + 0.5, 2.0) - 1.0);

    float shadow_strength = slope_strength * aspect_strength;
    float highlight_strength = slope_strength * (1.0 - aspect_strength);

    out_color = props.shadows[0] * shadow_strength + props.highlights[0] * highlight_strength;
}

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const HillshadeTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

    vec4 pixel = texture(image_sampler, frag_position);

    // Scale the derivative based on the mercator distortion at this latitude
    float scaleFactor = cos(radians((tileProps.latrange[0] - tileProps.latrange[1]) * frag_position.y + tileProps.latrange[1]));

    // The derivative is scaled back from [0, 1] texture range to world-space slope
    // Texture range [0, 1] corresponds to slope range [-4, 4]
    vec2 deriv = ((pixel.rg * 8.0) - 4.0) / scaleFactor;

    // Dispatch to the selected hillshade method
    if (tileProps.method == BASIC) {
        basic_hillshade(deriv, tileProps);
    } else if (tileProps.method == COMBINED) {
        combined_hillshade(deriv, tileProps);
    } else if (tileProps.method == IGOR) {
        igor_hillshade(deriv, tileProps);
    } else if (tileProps.method == MULTIDIRECTIONAL) {
        multidirectional_hillshade(deriv, tileProps);
    } else {
        // Default to STANDARD
        standard_hillshade(deriv, tileProps);
    }
}
)";
};

} // namespace shaders
} // namespace mbgl
