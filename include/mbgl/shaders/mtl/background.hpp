#pragma once

#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static constexpr AttributeInfo attributes[] = {
        {0, "a_pos", gfx::AttributeDataType::Float3, 1},
    };
    static constexpr UniformBlockInfo uniforms[] = {
        {1, sizeof(BackgroundLayerUBO), true, true, "BackgroundLayerUBO"},
        {2, sizeof(BackgroundDrawableUBO), true, false, "BackgroundDrawableUBO"},
    };

    static constexpr auto source = R"(
#include <metal_stdlib>
using namespace metal;

struct alignas(16) BackgroundDrawableUBO {
    float4x4 matrix;
};
struct alignas(16) BackgroundLayerUBO {
    float4 color;
    float opacity, pad1, pad2, pad3;
};

struct v2f {
    float4 position [[position]];
};

v2f vertex vertexMain(uint vertexId [[vertex_id]],
                      device const short2* positions [[buffer(0)]],
                      device const BackgroundLayerUBO& layerUBO [[buffer(1)]],
                      device const BackgroundDrawableUBO& drawableUBO [[buffer(2)]]) {
    return { drawableUBO.matrix * float4(positions[vertexId].x, positions[vertexId].y, 0.0, 1.0) };
}

half4 fragment fragmentMain(v2f in [[stage_in]],
                            device const BackgroundLayerUBO& layerUBO [[buffer(1)]]) {
#ifdef OVERDRAW_INSPECTOR
    return half4(1.0);
#else
    return half4(layerUBO.color) * layerUBO.opacity;
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
