struct VertexStage {
    short2 position [[attribute(0)]];
    float2 color [[attribute(1)]];
    float2 opacity [[attribute(2)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 color;
    half opacity;
};

struct alignas(16) FillDrawableUBO {
    float4x4 matrix;
};

struct alignas(16) FillEvaluatedPropsUBO {
    float4 color;
    float opacity;
};

struct alignas(16) FillInterpolateUBO {
    float color_t;
    float opacity_t;
};

struct alignas(16) FillPermutationUBO {
    Attribute color;
    Attribute opacity;
    bool overdrawInspector;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillDrawableUBO& drawable [[buffer(3)]],
                                device const FillEvaluatedPropsUBO& props [[buffer(4)]],
                                device const FillInterpolateUBO& interp [[buffer(5)]],
                                device const FillPermutationUBO& permutation [[buffer(6)]],
                                device const ExpressionInputsUBO& expr [[buffer(7)]]) {

    const auto color          = colorFor(permutation.color,          props.color,          vertx.color,                                   expr);
    const auto opacity        = valueFor(permutation.opacity,        props.opacity,        vertx.opacity,        interp.opacity_t,        expr);

    float4 position = drawable.matrix * float4(float2(vertx.position), 0.0f, 1.0f);

    return {
        .position       = position,
        .color          = color,
        .opacity        = half(opacity),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(4)]],
                            device const FillPermutationUBO& permutation [[buffer(6)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    return half4(in.color * in.opacity);
}
