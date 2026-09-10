struct VertexStage {
    float3 position [[attribute(0)]];
    float2 uv [[attribute(1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 uv;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const CustomGeometryDrawableUBO& drawable [[buffer(idCustomGeometryDrawableUBO)]]) {

    return {
        .position = drawable.matrix * float4(vertx.position, 1.0),
        .uv = vertx.uv
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const CustomGeometryDrawableUBO& drawable [[buffer(idCustomGeometryDrawableUBO)]],
                            texture2d<float, access::sample> colorTexture [[texture(0)]]) {
    constexpr sampler sampler2d(coord::normalized, filter::linear);
    const float4 color = colorTexture.sample(sampler2d, in.uv) * drawable.color;

    return half4(color);
}
