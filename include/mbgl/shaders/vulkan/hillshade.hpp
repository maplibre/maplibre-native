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
    frag_position.y = 1.0 - frag_position.y; // TODO check this. prepare should ignore the flip
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position;
layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct HillshadeTilePropsUBO {
    vec2 latrange;
    vec2 light;
};

layout(std140, set = LAYER_SET_INDEX, binding = idHillshadeTilePropsUBO) readonly buffer HillshadeTilePropsUBOVector {
    HillshadeTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idHillshadeEvaluatedPropsUBO) uniform HillshadeEvaluatedPropsUBO {
    vec4 highlight;
    vec4 shadow;
    vec4 accent;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const HillshadeTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

    vec4 pixel = texture(image_sampler, frag_position);

    vec2 deriv = pixel.rg * 2.0 - 1.0;

    // We divide the slope by a scale factor based on the cosin of the pixel's approximate latitude
    // to account for mercator projection distortion. see #4807 for details
    float scaleFactor = cos(radians((tileProps.latrange[0] - tileProps.latrange[1]) * frag_position.y + tileProps.latrange[1]));
    // We also multiply the slope by an arbitrary z-factor of 1.25
    float slope = atan(1.25 * length(deriv) / scaleFactor);
    float aspect = deriv.x != 0.0 ? atan(deriv.y, -deriv.x) : M_PI / 2.0 * (deriv.y > 0.0 ? 1.0 : -1.0);

    float intensity = tileProps.light.x;
    // We add PI to make this property match the global light object, which adds PI/2 to the light's azimuthal
    // position property to account for 0deg corresponding to north/the top of the viewport in the style spec
    // and the original shader was written to accept (-illuminationDirection - 90) as the azimuthal.
    float azimuth = tileProps.light.y + M_PI;

    // We scale the slope exponentially based on intensity, using a calculation similar to
    // the exponential interpolation function in the style spec:
    // https://github.com/mapbox/mapbox-gl-js/blob/master/src/style-spec/expression/definitions/interpolate.js#L217-L228
    // so that higher intensity values create more opaque hillshading.
    float base = 1.875 - intensity * 1.75;
    float maxValue = 0.5 * M_PI;
    float scaledSlope = intensity != 0.5 ? ((pow(base, slope) - 1.0) / (pow(base, maxValue) - 1.0)) * maxValue : slope;

    // The accent color is calculated with the cosine of the slope while the shade color is calculated with the sine
    // so that the accent color's rate of change eases in while the shade color's eases out.
    float accent = cos(scaledSlope);
    // We multiply both the accent and shade color by a clamped intensity value
    // so that intensities >= 0.5 do not additionally affect the color values
    // while intensity values < 0.5 make the overall color more transparent.
    vec4 accent_color = (1.0 - accent) * props.accent * clamp(intensity * 2.0, 0.0, 1.0);
    float shade = abs(mod((aspect + azimuth) / M_PI + 0.5, 2.0) - 1.0);
    vec4 shade_color = mix(props.shadow, props.highlight, shade) * sin(scaledSlope) * clamp(intensity * 2.0, 0.0, 1.0);
    vec4 color = accent_color * (1.0 - shade_color.a) + shade_color;

    out_color = color;
}
)";
};

} // namespace shaders
} // namespace mbgl
