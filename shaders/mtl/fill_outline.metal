struct VertexStage {
    short2 position [[attribute(0)]];
    float2 outline_color [[attribute(1)]];
    float2 opacity [[attribute(2)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 outline_color;
    float2 pos;
    half opacity;
};

struct alignas(16) FillOutlineDrawableUBO {
    float4x4 matrix;
    float2 world;
    float2 pad1;
};

struct alignas(16) FillOutlineEvaluatedPropsUBO {
    float4 outline_color;
    float opacity;
};

struct alignas(16) FillOutlineInterpolateUBO {
    float outline_color_t;
    float opacity_t;
};

struct alignas(16) FillOutlinePermutationUBO {
    Attribute outline_color;
    Attribute opacity;
    bool overdrawInspector;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillOutlineDrawableUBO& drawable [[buffer(3)]],
                                device const FillOutlineEvaluatedPropsUBO& props [[buffer(4)]],
                                device const FillOutlineInterpolateUBO& interp [[buffer(5)]],
                                device const FillOutlinePermutationUBO& permutation [[buffer(6)]],
                                device const ExpressionInputsUBO& expr [[buffer(7)]]) {

    const auto outline_color  = colorFor(permutation.outline_color,  props.outline_color,  vertx.outline_color,                           expr);
    const auto opacity        = valueFor(permutation.opacity,        props.opacity,        vertx.opacity,        interp.opacity_t,        expr);

    float4 position = drawable.matrix * float4(float2(vertx.position), 0.0f, 1.0f);
    float2 v_pos = (position.xy / position.w + 1.0) / 2.0 * drawable.world;

    return {
        .position       = position,
        .outline_color  = outline_color,
        .pos            = v_pos,
        .opacity        = half(opacity),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillOutlineEvaluatedPropsUBO& props [[buffer(4)]],
                            device const FillOutlinePermutationUBO& permutation [[buffer(6)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

//   TODO: Cause metal line primitive only support draw 1 pixel width line
//   use alpha to provide edge antialiased is no point
//   Should triangate the lines into triangles to support thick line and edge antialiased.
//    float dist = length(in.pos - in.position.xy);
//    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);

    return half4(in.outline_color * in.opacity);
}
