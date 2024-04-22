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

    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
};

struct alignas(16) HeatmapTexturePropsUBO {
    float4x4 matrix;
    float2 world;
    float opacity;
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const HeatmapTexturePropsUBO& props [[buffer(0)]]) {

    const float2 pos = float2(vertx.pos);
    const float4 position = props.matrix * float4(pos * props.world, 0, 1);

    return {
        .position    = position,
        .pos         = pos,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const HeatmapTexturePropsUBO& props [[buffer(0)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            texture2d<float, access::sample> color_ramp [[texture(1)]],
                            sampler image_sampler [[sampler(0)]],
                            sampler color_ramp_sampler [[sampler(1)]]) {

#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    float t = image.sample(image_sampler, in.pos).r;
    float4 color = color_ramp.sample(color_ramp_sampler, float2(t, 0.5));
    return half4(color * props.opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl
