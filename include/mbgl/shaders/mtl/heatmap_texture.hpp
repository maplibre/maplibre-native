#pragma once

#include <mbgl/shaders/heatmap_texture_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "HeatmapTextureShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = false;

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
};

struct alignas(16) HeatmapTextureDrawableUBO {
    float4x4 matrix;
    float2 world;
    float opacity;
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const HeatmapTextureDrawableUBO& drawable [[buffer(1)]]) {

    const float2 pos = float2(vertx.pos);
    const float4 position = drawable.matrix * float4(pos * drawable.world, 0, 1);

    return {
        .position    = position,
        .pos         = pos,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const HeatmapTextureDrawableUBO& drawable [[buffer(1)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            texture2d<float, access::sample> color_ramp [[texture(1)]],
                            sampler image_sampler [[sampler(0)]],
                            sampler color_ramp_sampler [[sampler(1)]]) {

    if (drawable.overdrawInspector) {
        return half4(0.0);
    }

    float t = image.sample(image_sampler, in.pos).r;
    float4 color = color_ramp.sample(color_ramp_sampler, float2(t, 0.5));
    return half4(color * drawable.opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl
