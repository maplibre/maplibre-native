#pragma once

#include <mbgl/shaders/color_relief_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto colorReliefShaderPrelude = R"(

enum {
    idColorReliefDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idColorReliefTilePropsUBO = idDrawableReservedFragmentOnlyUBO,
    idColorReliefEvaluatedPropsUBO = drawableReservedUBOCount,
    colorReliefUBOCount
};

struct alignas(16) ColorReliefDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(ColorReliefDrawableUBO) == 4 * 16, "wrong size");

struct alignas(16) ColorReliefTilePropsUBO {
    /*  0 */ float4 unpack;
    /* 16 */ float2 dimension;
    /* 24 */ int32_t color_ramp_size;
    /* 28 */ float pad0;
    /* 32 */
};
static_assert(sizeof(ColorReliefTilePropsUBO) == 2 * 16, "wrong size");

struct alignas(16) ColorReliefEvaluatedPropsUBO {
    /*  0 */ float opacity;
    /*  4 */ float pad0;
    /*  8 */ float pad1;
    /* 12 */ float pad2;
    /* 16 */
};
static_assert(sizeof(ColorReliefEvaluatedPropsUBO) == 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "ColorReliefShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 3> textures;

    static constexpr auto prelude = colorReliefShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(colorReliefUBOCount + 0)]];
    short2 texture_pos [[attribute(colorReliefUBOCount + 1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const ColorReliefDrawableUBO* drawableVector [[buffer(idColorReliefDrawableUBO)]]) {

    device const ColorReliefDrawableUBO& drawable = drawableVector[uboIndex];

    const float4 position = drawable.matrix * float4(float2(vertx.pos), 0, 1);
    float2 pos = float2(vertx.texture_pos) / 8192.0;
    pos.y = 1.0 - pos.y;

    return {
        .position    = position,
        .pos         = pos,
    };
}

float getElevation(float2 coord, texture2d<float, access::sample> image, sampler image_sampler, float4 unpack) {
    float4 data = image.sample(image_sampler, coord) * 255.0;
    data.a = -1.0;
    return dot(data, unpack);
}

float4 getColorForElevation(float elevation,
                           int numStops,
                           texture2d<float, access::sample> elevationStops,
                           texture2d<float, access::sample> colorStops,
                           sampler stops_sampler) {
    // Binary search for the correct elevation range
    int low = 0;
    int high = numStops - 1;

    while (low < high) {
        int mid = (low + high) / 2;
        float midElevation = getElevation(float2((float(mid) + 0.5) / float(numStops), 0.5),
                                         elevationStops, stops_sampler, float4(1.0, 1.0, 1.0, 1.0));

        if (elevation < midElevation) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    // Get the two surrounding stops
    int idx1 = max(low - 1, 0);
    int idx2 = min(low, numStops - 1);

    float elev1 = getElevation(float2((float(idx1) + 0.5) / float(numStops), 0.5),
                              elevationStops, stops_sampler, float4(1.0, 1.0, 1.0, 1.0));
    float elev2 = getElevation(float2((float(idx2) + 0.5) / float(numStops), 0.5),
                              elevationStops, stops_sampler, float4(1.0, 1.0, 1.0, 1.0));

    float4 color1 = colorStops.sample(stops_sampler, float2((float(idx1) + 0.5) / float(numStops), 0.5));
    float4 color2 = colorStops.sample(stops_sampler, float2((float(idx2) + 0.5) / float(numStops), 0.5));

    // Interpolate between the two colors
    float t = (elev2 - elev1) > 0.0 ? clamp((elevation - elev1) / (elev2 - elev1), 0.0, 1.0) : 0.0;
    return mix(color1, color2, t);
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const ColorReliefTilePropsUBO* tilePropsVector [[buffer(idColorReliefTilePropsUBO)]],
                            device const ColorReliefEvaluatedPropsUBO& props [[buffer(idColorReliefEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            texture2d<float, access::sample> elevationStops [[texture(1)]],
                            texture2d<float, access::sample> colorStops [[texture(2)]],
                            sampler image_sampler [[sampler(0)]],
                            sampler stops_sampler [[sampler(1)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const ColorReliefTilePropsUBO& tileProps = tilePropsVector[uboIndex];

    // Get elevation at this pixel
    float elevation = getElevation(in.pos, image, image_sampler, tileProps.unpack);

    // Look up color from color ramp
    float4 color = getColorForElevation(elevation,
                                       tileProps.color_ramp_size,
                                       elevationStops,
                                       colorStops,
                                       stops_sampler);

    // Apply opacity
    color.a *= props.opacity;

    return half4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
