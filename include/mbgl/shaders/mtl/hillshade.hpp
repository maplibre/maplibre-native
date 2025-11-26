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

struct alignas(16) HillshadeDrawableUBO {
    /* 0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(HillshadeDrawableUBO) == 4 * 16, "wrong size");

struct alignas(16) HillshadeTilePropsUBO {
    /* 0 */ float latrange;
    /* 4 */ float padding;
    /* 8 */ float2 light;
    /* 16 */
};
static_assert(sizeof(HillshadeTilePropsUBO) == 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) HillshadeEvaluatedPropsUBO {
    /* 0 */ float4 highlight;
    /* 16 */ float4 shadow;
    /* 32 */ float4 accent;
    /* 48 */
};
static_assert(sizeof(HillshadeEvaluatedPropsUBO) == 3 * 16, "wrong size");

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
    pos.y = 1.0 - pos.y;

    return {
        .position    = position,
        .pos         = pos,
    };
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

    float4 pixel = image.sample(image_sampler, in.pos);

    float2 deriv = ((pixel.rg * 2.0) - 1.0);

    // We divide the slope by a scale factor based on the cosin of the pixel's approximate latitude
    // to account for mercator projection distortion. see #4807 for details
    float scaleFactor = cos(radians((tileProps.latrange[0] - tileProps.latrange[1]) * in.pos.y + tileProps.latrange[1]));
    // We also multiply the slope by an arbitrary z-factor of 1.25
    float slope = atan(1.25 * length(deriv) / scaleFactor);
    float aspect = deriv.x != 0.0 ? atan2(deriv.y, -deriv.x) : M_PI_F / 2.0 * (deriv.y > 0.0 ? 1.0 : -1.0);

    float intensity = tileProps.light.x;
    // We add PI to make this property match the global light object, which adds PI/2 to the light's azimuthal
    // position property to account for 0deg corresponding to north/the top of the viewport in the style spec
    // and the original shader was written to accept (-illuminationDirection - 90) as the azimuthal.
    float azimuth = tileProps.light.y + M_PI_F;

    // We scale the slope exponentially based on intensity, using a calculation similar to
    // the exponential interpolation function in the style spec:
    // https://github.com/mapbox/mapbox-gl-js/blob/master/src/style-spec/expression/definitions/interpolate.js#L217-L228
    // so that higher intensity values create more opaque hillshading.
    float base = 1.875 - intensity * 1.75;
    float maxValue = 0.5 * M_PI_F;
    float scaledSlope = intensity != 0.5 ? ((pow(base, slope) - 1.0) / (pow(base, maxValue) - 1.0)) * maxValue : slope;

    // The accent color is calculated with the cosine of the slope while the shade color is calculated with the sine
    // so that the accent color's rate of change eases in while the shade color's eases out.
    float accent = cos(scaledSlope);
    // We multiply both the accent and shade color by a clamped intensity value
    // so that intensities >= 0.5 do not additionally affect the color values
    // while intensity values < 0.5 make the overall color more transparent.
    float4 accent_color = (1.0 - accent) * props.accent * clamp(intensity * 2.0, 0.0, 1.0);
    float shade = abs(glMod((aspect + azimuth) / M_PI_F + 0.5, 2.0) - 1.0);
    float4 shade_color = mix(props.shadow, props.highlight, shade) * sin(scaledSlope) * clamp(intensity * 2.0, 0.0, 1.0);
    float4 color = accent_color * (1.0 - shade_color.a) + shade_color;

    return half4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
