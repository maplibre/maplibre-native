#pragma once

#include <mbgl/shaders/debug_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto debugShaderPrelude = R"(

enum {
    idDebugUBO = drawableReservedUBOCount,
    debugUBOCount
};

struct alignas(16) DebugUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float4 color;
    /* 80 */ float overlay_scale;
    /* 84 */ float pad1;
    /* 88 */ float pad2;
    /* 92 */ float pad3;
    /* 96 */
};
static_assert(sizeof(DebugUBO) == 6 * 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::DebugShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "DebugShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = debugShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(debugUBOCount + 0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 uv;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const DebugUBO& debug [[buffer(idDebugUBO)]]) {

    const float4 position = debug.matrix * float4(float2(vertx.pos) * debug.overlay_scale, 0, 1);

    // This vertex shader expects a EXTENT x EXTENT quad,
    // The UV co-ordinates for the overlay texture can be calculated using that knowledge
    float2 uv = float2(vertx.pos) / 8192.0;

    return {
        .position    = position,
        .uv          = uv
    };
}

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const DebugUBO& debug [[buffer(idDebugUBO)]],
                            texture2d<float, access::sample> overlay [[texture(0)]],
                            sampler overlay_sampler [[sampler(0)]]) {

    float4 overlay_color = overlay.sample(overlay_sampler, in.uv);
    float4 color = mix(debug.color, overlay_color, overlay_color.a);
    return PrecisionFloat4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
