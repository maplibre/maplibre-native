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
    /* 28 */ float pad_tile0;
    /* 32 */
};
static_assert(sizeof(ColorReliefTilePropsUBO) == 2 * 16, "wrong size");

struct alignas(16) ColorReliefEvaluatedPropsUBO {
    /*  0 */ float opacity;
    /*  4 */ float pad_eval0;
    /*  8 */ float pad_eval1;
    /* 12 */ float pad_eval2;
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

// Get elevation from DEM texture
float getElevation(float2 coord, texture2d<float, access::sample> image, sampler image_sampler, float4 unpack) {
    float4 data = image.sample(image_sampler, coord) * 255.0;
    data.a = -1.0;
    return dot(data, unpack);
}

// Get elevation stop value from texture (raw float, not encoded)
float getElevationStop(int index, int numStops, texture2d<float, access::sample> elevationStops, sampler stops_sampler) {
    float x = (float(index) + 0.5) / float(numStops);
    return elevationStops.sample(stops_sampler, float2(x, 0.5)).r;
}

// Get color stop from texture
float4 getColorStop(int index, int numStops, texture2d<float, access::sample> colorStops, sampler stops_sampler) {
    float x = (float(index) + 0.5) / float(numStops);
    return colorStops.sample(stops_sampler, float2(x, 0.5));
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

    // Get elevation at this pixel from DEM
    float el = getElevation(in.pos, image, image_sampler, tileProps.unpack);

    // Binary search to find surrounding elevation stops
    int l = 0;
    int r = tileProps.color_ramp_size - 1;
    
    while (r - l > 1) {
        int m = (r + l) / 2;
        float el_m = getElevationStop(m, tileProps.color_ramp_size, elevationStops, stops_sampler);
        
        if (el < el_m) {
            r = m;
        } else {
            l = m;
        }
    }

    // Get the elevation values at the stops
    float el_l = getElevationStop(l, tileProps.color_ramp_size, elevationStops, stops_sampler);
    float el_r = getElevationStop(r, tileProps.color_ramp_size, elevationStops, stops_sampler);

    // Get the colors at the stops
    float4 color_l = getColorStop(l, tileProps.color_ramp_size, colorStops, stops_sampler);
    float4 color_r = getColorStop(r, tileProps.color_ramp_size, colorStops, stops_sampler);

    // Interpolate
    float t = clamp((el - el_l) / (el_r - el_l), 0.0, 1.0);
    float4 color = mix(color_l, color_r, t);

    // Apply opacity
    color.a *= props.opacity;

    return half4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
