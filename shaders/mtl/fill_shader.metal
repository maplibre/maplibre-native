struct VertexStage {
    short2 position [[attribute(0)]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color [[attribute(1)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(2)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];

#if !defined(HAS_UNIFORM_u_color)
    half4 color;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillDrawableUnionUBO* drawableVector [[buffer(idFillDrawableUBO)]]) {

    device const FillDrawableUBO& drawable = drawableVector[uboIndex].fillDrawableUBO;

    return {
        .position = drawable.matrix * float4(float2(vertx.position), 0.0f, 1.0f),
#if !defined(HAS_UNIFORM_u_color)
        .color    = half4(unpack_mix_color(vertx.color, drawable.color_t)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity  = half(unpack_mix_float(vertx.opacity, drawable.opacity_t)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

#if defined(HAS_UNIFORM_u_color)
    const half4 color = half4(props.color);
#else
    const half4 color = in.color;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const half opacity = props.opacity;
#else
    const half opacity = in.opacity;
#endif

    return half4(color * opacity);
}
