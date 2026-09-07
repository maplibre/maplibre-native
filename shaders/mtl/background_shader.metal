

struct VertexStage {
    short2 position [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const BackgroundDrawableUnionUBO* drawableVector [[buffer(idBackgroundDrawableUBO)]]) {

    device const BackgroundDrawableUBO& drawable = drawableVector[uboIndex].backgroundDrawableUBO;

    return {
        .position = drawable.matrix * float4(float2(in.position.xy), 0, 1)
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const BackgroundPropsUBO& props [[buffer(idBackgroundPropsUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    return half4(props.color * props.opacity);
}
