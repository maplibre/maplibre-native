#include <metal_stdlib>
using namespace metal;

struct VertexStage {
    short2 position [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

struct alignas(16) BackgroundDrawableUBO {
    float4x4 matrix;
};
struct alignas(16) BackgroundLayerUBO {
    float4 color;
    float opacity, pad1, pad2, pad3;
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const BackgroundLayerUBO& layerUBO [[buffer(1)]],
                                device const BackgroundDrawableUBO& drawableUBO [[buffer(2)]]) {
    return {
        .position = drawableUBO.matrix * float4(float2(in.position.xy), 0, 1)
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const BackgroundLayerUBO& layerUBO [[buffer(1)]]) {
#ifdef OVERDRAW_INSPECTOR
    return half4(1.0);
#else
    return half4(float4(layerUBO.color.rgb, layerUBO.color.a * layerUBO.opacity));
#endif
}