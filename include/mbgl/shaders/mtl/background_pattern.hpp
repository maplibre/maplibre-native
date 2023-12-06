#pragma once

#include <mbgl/shaders/mtl/background.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundPatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<UniformBlockInfo, 2> uniforms;
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexStage {
    short2 position [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    half2 pos_a;
    half2 pos_b;
};

struct alignas(16) BackgroundDrawableUBO {
    float4x4 matrix;
};
struct alignas(16) BackgroundLayerUBO {
    float2 pattern_tl_a;
    float2 pattern_br_a;
    float2 pattern_tl_b;
    float2 pattern_br_b;
    float2 texsize;
    float2 pattern_size_a;
    float2 pattern_size_b;
    float2 pixel_coord_upper;
    float2 pixel_coord_lower;
    float tile_units_to_pixels;
    float scale_a;
    float scale_b;
    float mix;
    float opacity;
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const BackgroundDrawableUBO& drawableUBO [[buffer(1)]],
                                device const BackgroundLayerUBO& layerUBO [[buffer(2)]]) {
    const float2 pos = float2(in.position);
    const float2 pos_a = get_pattern_pos(layerUBO.pixel_coord_upper,
                                         layerUBO.pixel_coord_lower,
                                         layerUBO.scale_a * layerUBO.pattern_size_a,
                                         layerUBO.tile_units_to_pixels,
                                         pos);
    const float2 pos_b = get_pattern_pos(layerUBO.pixel_coord_upper,
                                         layerUBO.pixel_coord_lower,
                                         layerUBO.scale_b * layerUBO.pattern_size_b,
                                         layerUBO.tile_units_to_pixels,
                                         pos);
    return {
        .position = drawableUBO.matrix * float4(float2(in.position.xy), 0, 1),
        .pos_a = half2(glMod(pos_a, 1.0)),
        .pos_b = half2(glMod(pos_b, 1.0)),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const BackgroundLayerUBO& layerUBO [[buffer(2)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    const float2 pos = mix(layerUBO.pattern_tl_a / layerUBO.texsize, layerUBO.pattern_br_a / layerUBO.texsize, float2(in.pos_a));
    const float4 color1 = image.sample(image_sampler, pos);

    const float2 pos2 = mix(layerUBO.pattern_tl_b / layerUBO.texsize, layerUBO.pattern_br_b / layerUBO.texsize, float2(in.pos_b));
    const float4 color2 = image.sample(image_sampler, pos2);

    return half4(mix(color1, color2, layerUBO.mix) * layerUBO.opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl
