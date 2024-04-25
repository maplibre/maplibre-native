#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<UniformBlockInfo, 2> uniforms;
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexStage {
    short2 position [[attribute(3)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

struct alignas(16) BackgroundDrawableUBO {
    float4x4 matrix;
};
struct alignas(16) BackgroundLayerUBO {
    float4 color;
    float opacity;
    float pad1, pad2, pad3;
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const BackgroundDrawableUBO& drawableUBO [[buffer(1)]]) {
    return {
        .position = drawableUBO.matrix * float4(float2(in.position.xy), 0, 1)
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const BackgroundLayerUBO& layerUBO [[buffer(2)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    return half4(layerUBO.color * layerUBO.opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl
