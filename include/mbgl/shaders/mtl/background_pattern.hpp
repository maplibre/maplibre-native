#pragma once

#include <mbgl/shaders/mtl/background.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundPatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexStage {
    short2 position [[attribute(3)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos_a;
    float2 pos_b;
};

struct alignas(16) BackgroundPatternDrawableUBO {
    float4x4 matrix;
    float2 pixel_coord_upper;
    float2 pixel_coord_lower;
    float tile_units_to_pixels;
    float pad1, pad2, pad3;
};
struct alignas(16) BackgroundPatternLayerUBO {
    float2 pattern_tl_a;
    float2 pattern_br_a;
    float2 pattern_tl_b;
    float2 pattern_br_b;
    float2 pattern_size_a;
    float2 pattern_size_b;
    float scale_a;
    float scale_b;
    float mix;
    float opacity;
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const BackgroundPatternDrawableUBO& drawableUBO [[buffer(1)]],
                                device const BackgroundPatternLayerUBO& layerUBO [[buffer(2)]]) {
    const float2 pos = float2(in.position);
    const float2 pos_a = get_pattern_pos(drawableUBO.pixel_coord_upper,
                                         drawableUBO.pixel_coord_lower,
                                         layerUBO.scale_a * layerUBO.pattern_size_a,
                                         drawableUBO.tile_units_to_pixels,
                                         pos);
    const float2 pos_b = get_pattern_pos(drawableUBO.pixel_coord_upper,
                                         drawableUBO.pixel_coord_lower,
                                         layerUBO.scale_b * layerUBO.pattern_size_b,
                                         drawableUBO.tile_units_to_pixels,
                                         pos);
    return {
        .position = drawableUBO.matrix * float4(float2(in.position.xy), 0, 1),
        .pos_a = pos_a,
        .pos_b = pos_b,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParamsUBO [[buffer(0)]],
                            device const BackgroundPatternLayerUBO& layerUBO [[buffer(2)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    const float2 texsize = paintParamsUBO.pattern_atlas_texsize;
    const float2 imagecoord = glMod(float2(in.pos_a), 1.0);
    const float2 pos = mix(layerUBO.pattern_tl_a / texsize, layerUBO.pattern_br_a / texsize, imagecoord);
    const float4 color1 = image.sample(image_sampler, pos);
    const float2 imagecoord_b = glMod(float2(in.pos_b), 1.0);
    const float2 pos2 = mix(layerUBO.pattern_tl_b / texsize, layerUBO.pattern_br_b / texsize, imagecoord_b);
    const float4 color2 = image.sample(image_sampler, pos2);

    return half4(mix(color1, color2, layerUBO.mix) * layerUBO.opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl
