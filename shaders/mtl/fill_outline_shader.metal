struct VertexStage {
    short2 position [[attribute(0)]];
    float4 outline_color [[attribute(1)]];
    float2 opacity [[attribute(2)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
#if !defined(HAS_UNIFORM_u_outline_color)
    half4 outline_color;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillDrawableUnionUBO* drawableVector [[buffer(idFillDrawableUBO)]]) {

    device const FillOutlineDrawableUBO& drawable = drawableVector[uboIndex].fillOutlineDrawableUBO;

    const float4 position = drawable.matrix * float4(float2(vertx.position), 0.0f, 1.0f);
    return {
        .position       = position,
        .pos            = (position.xy / position.w + 1.0) / 2.0 * paintParams.world_size,
#if !defined(HAS_UNIFORM_u_outline_color)
        .outline_color  = half4(unpack_mix_color(vertx.outline_color, drawable.outline_color_t)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity        = half(unpack_mix_float(vertx.opacity, drawable.opacity_t)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

//   TODO: Cause metal line primitive only support draw 1 pixel width line
//   use alpha to provide edge antialiased is no point
//   Should triangate the lines into triangles to support thick line and edge antialiased.
//    float dist = length(in.pos - in.position.xy);
//    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);

#if defined(HAS_UNIFORM_u_outline_color)
    const half4 color = half4(props.outline_color);
#else
    const half4 color = in.outline_color;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const half opacity = props.opacity;
#else
    const half opacity = in.opacity;
#endif

    return half4(color * opacity);
}
