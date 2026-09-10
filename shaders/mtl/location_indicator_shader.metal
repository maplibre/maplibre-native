struct VertexStage {
    float2 position [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const LocationIndicatorDrawableUBO& drawable [[buffer(idLocationIndicatorUBO)]]) {

    return {
        .position = drawable.matrix * float4(vertx.position, 1.0)
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const LocationIndicatorDrawableUBO& drawable [[buffer(idLocationIndicatorUBO)]]) {
    return half4(drawable.color);
}
