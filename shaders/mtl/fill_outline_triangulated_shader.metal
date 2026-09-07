

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float width2;
    float2 normal;
    half gamma_scale;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillDrawableUnionUBO* drawableVector [[buffer(idFillDrawableUBO)]]) {

    device const FillOutlineTriangulatedDrawableUBO& drawable = drawableVector[uboIndex].fillOutlineTriangulatedDrawableUBO;

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float2 pos = floor(float2(vertx.pos_normal) * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of a_pos_normal
    const float2 normal = float2(vertx.pos_normal) - 2.0 * pos;
    const float2 v_normal = float2(normal.x, normal.y * 2.0 - 1.0);

    const float width = 1.0; //Should come from props ubo
    const float halfwidth = width / 2.0;
    const float outset = halfwidth + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width of this vertex.
    const float2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    const float4 projected_extrude = drawable.matrix * float4(dist / drawable.ratio, 0.0, 0.0);
    const float4 position = drawable.matrix * float4(pos, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    return {
        .position    = position,
        .width2      = outset,
        .normal      = v_normal,
        .gamma_scale = half(extrude_length_without_perspective / extrude_length_with_perspective),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]]) {

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (`v_width2.y`) or when fading out (`v_width2.x`)
    const float blur2 = (1.0 / DEVICE_PIXEL_RATIO) * in.gamma_scale;
    const float alpha = clamp(min(dist + blur2, in.width2 - dist) / blur2, 0.0, 1.0);

    return half4(props.outline_color * (alpha * props.opacity));
}
