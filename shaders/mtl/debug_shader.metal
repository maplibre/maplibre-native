struct VertexStage {
    short2 pos [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 uv;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const DebugUBO& debug [[buffer(idDebugUBO)]]) {

    const float4 position = debug.matrix * float4(float2(vertx.pos) * debug.overlay_scale, 0, 1);

    // This vertex shader expects a EXTENT x EXTENT quad,
    // The UV coordinates for the overlay texture can be calculated using that knowledge
    float2 uv = float2(vertx.pos) / 8192.0;

    return {
        .position    = position,
        .uv          = uv
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const DebugUBO& debug [[buffer(idDebugUBO)]],
                            texture2d<float, access::sample> overlay [[texture(0)]],
                            sampler overlay_sampler [[sampler(0)]]) {

    float4 overlay_color = overlay.sample(overlay_sampler, in.uv);
    float4 color = mix(debug.color, overlay_color, overlay_color.a);
    return half4(color);
}
