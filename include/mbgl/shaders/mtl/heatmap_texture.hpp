#pragma once

#include <mbgl/shaders/heatmap_texture_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

#define HEATMAP_TEXTURE_SHADER_PRELUDE R"(

struct alignas(16) HeatmapTexturePropsUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float opacity;
    /* 68 */ float pad1;
    /* 72 */ float pad2;
    /* 76 */ float pad3;
    /* 80 */
};
static_assert(sizeof(HeatmapTexturePropsUBO) == 5 * 16, "wrong size");

enum {
    idHeatmapTexturePropsUBO = globalUBOCount,
    heatmapTextureUBOCount
};

)"

template <>
struct ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "HeatmapTextureShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<UniformBlockInfo, 2> uniforms;
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto source = HEATMAP_TEXTURE_SHADER_PRELUDE R"(

struct VertexStage {
    short2 pos [[attribute(heatmapTextureUBOCount + 0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const HeatmapTexturePropsUBO& props [[buffer(idHeatmapTexturePropsUBO)]]) {

    const float2 pos = float2(vertx.pos);
    const float4 position = props.matrix * float4(pos * paintParams.world_size, 0, 1);

    return {
        .position    = position,
        .pos         = pos,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const HeatmapTexturePropsUBO& props [[buffer(idHeatmapTexturePropsUBO)]],
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
