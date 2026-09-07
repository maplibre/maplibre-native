

struct VertexStage {
    short2 position [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

struct FragmentResult {
    // color output is only needed because we're using implicit stencil writes
    half4 color [[color(0)]];
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const ClipUBO& clipUBO [[buffer(idClippingMaskUBO)]]) {
    return { clipUBO.matrix * float4(float2(in.position.xy), 0, 1) };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]]) {
    return half4(1.0);
}
