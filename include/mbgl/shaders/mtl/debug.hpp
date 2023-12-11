#pragma once

#include <mbgl/shaders/debug_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "DebugShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 uv;
};

struct alignas(16) DebugUBO {
    float4x4 matrix;
    float4 color;
    float overlay_scale;
    float pad1, pad2, pad3;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const DebugUBO& debugUBO [[buffer(1)]]) {

    const float4 position = debugUBO.matrix * float4(float2(vertx.pos) * debugUBO.overlay_scale, 0, 1);

    // This vertex shader expects a EXTENT x EXTENT quad,
    // The UV co-ordinates for the overlay texture can be calculated using that knowledge
    float2 uv = float2(vertx.pos) / 8192.0;

    return {
        .position    = position,
        .uv          = uv
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const DebugUBO& debugUBO [[buffer(1)]],
                            texture2d<float, access::sample> overlay [[texture(0)]],
                            sampler overlay_sampler [[sampler(0)]]) {

    float4 overlay_color = overlay.sample(overlay_sampler, in.uv);
    float4 color = mix(debugUBO.color, overlay_color, overlay_color.a);
    return half4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
