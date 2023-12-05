#pragma once

#include <mbgl/shaders/raster_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::RasterShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "RasterShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = false;

    static const std::array<AttributeInfo, 2> attributes;
    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(0)]];
    short2 texture_pos [[attribute(1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos0;
    float2 pos1;
};

struct alignas(16) RasterDrawableUBO {
    float4x4 matrix;
    float3 spin_weights;
    float2 tl_parent;
    float scale_parent;
    float buffer_scale;
    float fade_t;
    float opacity;
    float brightness_low;
    float brightness_high;
    float saturation_factor;
    float contrast_factor;
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
    float pad4;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const RasterDrawableUBO& drawable [[buffer(2)]]) {

    const float4 position = drawable.matrix * float4(float2(vertx.pos), 0, 1);

    // We are using Int16 for texture position coordinates to give us enough precision for
    // fractional coordinates. We use 8192 to scale the texture coordinates in the buffer
    // as an arbitrarily high number to preserve adequate precision when rendering.
    // This is also the same value as the EXTENT we are using for our tile buffer pos coordinates,
    // so math for modifying either is consistent.
    const float2 pos0 = (((float2(vertx.texture_pos) / 8192.0) - 0.5) / drawable.buffer_scale ) + 0.5;
    const float2 pos1 = (pos0 * drawable.scale_parent) + drawable.tl_parent;

    return {
        .position    = position,
        .pos0        = pos0,
        .pos1        = pos1,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const RasterDrawableUBO& drawable [[buffer(2)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            texture2d<float, access::sample> image1 [[texture(1)]],
                            sampler image0_sampler [[sampler(0)]],
                            sampler image1_sampler [[sampler(1)]]) {

    if (drawable.overdrawInspector) {
        return half4(1.0);
    }

    // read and cross-fade colors from the main and parent tiles
    float4 color0 = image0.sample(image0_sampler, in.pos0);
    float4 color1 = image1.sample(image1_sampler, in.pos1);
    if (color0.a > 0.0) {
        color0.rgb = color0.rgb / color0.a;
    }
    if (color1.a > 0.0) {
        color1.rgb = color1.rgb / color1.a;
    }
    float4 color = mix(color0, color1, drawable.fade_t);
    color.a *= drawable.opacity;
    float3 rgb = color.rgb;

    // spin
    rgb = float3(
        dot(rgb, drawable.spin_weights.xyz),
        dot(rgb, drawable.spin_weights.zxy),
        dot(rgb, drawable.spin_weights.yzx));

    // saturation
    float average = (color.r + color.g + color.b) / 3.0;
    rgb += (average - rgb) * drawable.saturation_factor;

    // contrast
    rgb = (rgb - 0.5) * drawable.contrast_factor + 0.5;

    // brightness
    float3 high_vec = float3(drawable.brightness_low, drawable.brightness_low, drawable.brightness_low);
    float3 low_vec = float3(drawable.brightness_high, drawable.brightness_high, drawable.brightness_high);

    return half4(half3(mix(high_vec, low_vec, rgb) * color.a), color.a);
}
)";
};

} // namespace shaders
} // namespace mbgl
