#pragma once

#include <mbgl/shaders/raster_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto rasterShaderPrelude = R"(

enum {
    idRasterDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idRasterEvaluatedPropsUBO = drawableReservedUBOCount,
    rasterUBOCount
};

struct alignas(16) RasterDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */
};
static_assert(sizeof(RasterDrawableUBO) == 4 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) RasterEvaluatedPropsUBO {
    /*  0 */ float4 spin_weights;
    /* 16 */ float2 tl_parent;
    /* 24 */ float scale_parent;
    /* 28 */ float buffer_scale;
    /* 32 */ float fade_t;
    /* 36 */ float opacity;
    /* 40 */ float brightness_low;
    /* 44 */ float brightness_high;
    /* 48 */ float saturation_factor;
    /* 52 */ float contrast_factor;
    /* 56 */ float pad1;
    /* 60 */ float pad2;
    /* 64 */
};
static_assert(sizeof(RasterEvaluatedPropsUBO) == 4 * 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "RasterShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto prelude = rasterShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(rasterUBOCount + 0)]];
    short2 texture_pos [[attribute(rasterUBOCount + 1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos0;
    float2 pos1;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const RasterDrawableUBO* drawableVector [[buffer(idRasterDrawableUBO)]],
                                device const RasterEvaluatedPropsUBO& props [[buffer(idRasterEvaluatedPropsUBO)]]) {

    device const RasterDrawableUBO& drawable = drawableVector[uboIndex];

    const float4 position = drawable.matrix * float4(float2(vertx.pos), 0, 1);

    // We are using Int16 for texture position coordinates to give us enough precision for
    // fractional coordinates. We use 8192 to scale the texture coordinates in the buffer
    // as an arbitrarily high number to preserve adequate precision when rendering.
    // This is also the same value as the EXTENT we are using for our tile buffer pos coordinates,
    // so math for modifying either is consistent.
    const float2 pos0 = (((float2(vertx.texture_pos) / 8192.0) - 0.5) / props.buffer_scale ) + 0.5;
    const float2 pos1 = (pos0 * props.scale_parent) + props.tl_parent;

    return {
        .position    = position,
        .pos0        = pos0,
        .pos1        = pos1,
    };
}

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const RasterEvaluatedPropsUBO& props [[buffer(idRasterEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            texture2d<float, access::sample> image1 [[texture(1)]],
                            sampler image0_sampler [[sampler(0)]],
                            sampler image1_sampler [[sampler(1)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return PrecisionFloat4(1.0);
#endif

    // read and cross-fade colors from the main and parent tiles
    float4 color0 = image0.sample(image0_sampler, in.pos0);
    float4 color1 = image1.sample(image1_sampler, in.pos1);
    if (color0.a > 0.0) {
        color0.rgb = color0.rgb / color0.a;
    }
    if (color1.a > 0.0) {
        color1.rgb = color1.rgb / color1.a;
    }
    float4 color = mix(color0, color1, props.fade_t);
    color.a *= props.opacity;
    float3 rgb = color.rgb;

    // spin
    rgb = float3(
        dot(rgb, props.spin_weights.xyz),
        dot(rgb, props.spin_weights.zxy),
        dot(rgb, props.spin_weights.yzx));

    // saturation
    float average = (color.r + color.g + color.b) / 3.0;
    rgb += (average - rgb) * props.saturation_factor;

    // contrast
    rgb = (rgb - 0.5) * props.contrast_factor + 0.5;

    // brightness
    float3 high_vec = float3(props.brightness_low, props.brightness_low, props.brightness_low);
    float3 low_vec = float3(props.brightness_high, props.brightness_high, props.brightness_high);

    return PrecisionFloat4(PrecisionFloat3(mix(high_vec, low_vec, rgb) * color.a), color.a);
}
)";
};

} // namespace shaders
} // namespace mbgl
