struct VertexStage {
    short2 position [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos_a;
    float2 pos_b;
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const BackgroundDrawableUnionUBO* drawableVector [[buffer(idBackgroundDrawableUBO)]],
                                device const BackgroundPatternPropsUBO& props [[buffer(idBackgroundPropsUBO)]]) {

    device const BackgroundPatternDrawableUBO& drawable = drawableVector[uboIndex].backgroundPatternDrawableUBO;

    const float2 pos = float2(in.position);
    const float2 pos_a = get_pattern_pos(drawable.pixel_coord_upper,
                                         drawable.pixel_coord_lower,
                                         props.scale_a * props.pattern_size_a,
                                         drawable.tile_units_to_pixels,
                                         pos);
    const float2 pos_b = get_pattern_pos(drawable.pixel_coord_upper,
                                         drawable.pixel_coord_lower,
                                         props.scale_b * props.pattern_size_b,
                                         drawable.tile_units_to_pixels,
                                         pos);
    return {
        .position = drawable.matrix * float4(float2(in.position.xy), 0, 1),
        .pos_a = pos_a,
        .pos_b = pos_b,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParamsUBO [[buffer(idGlobalPaintParamsUBO)]],
                            device const BackgroundPatternPropsUBO& props [[buffer(idBackgroundPropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    const float2 texsize = paintParamsUBO.pattern_atlas_texsize;
    const float2 imagecoord = glMod(float2(in.pos_a), 1.0);
    const float2 pos = mix(props.pattern_tl_a / texsize, props.pattern_br_a / texsize, imagecoord);
    const float4 color1 = image.sample(image_sampler, pos);
    const float2 imagecoord_b = glMod(float2(in.pos_b), 1.0);
    const float2 pos2 = mix(props.pattern_tl_b / texsize, props.pattern_br_b / texsize, imagecoord_b);
    const float4 color2 = image.sample(image_sampler, pos2);

    return half4(mix(color1, color2, props.mix) * props.opacity);
}
