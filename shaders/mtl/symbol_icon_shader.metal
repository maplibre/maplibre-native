struct VertexStage {
    float2 pos [[attribute(0)]];

#if !defined(HAS_UNIFORM_u_sorted_instance)
    ushort sorted_instance [[attribute(1)]];
#endif
};

struct SymbolInstance {
    short2 pos_scale[2];
    short2 offset_tltr[2];
    short2 offset_blbr[2];
    ushort2 texture_rect[2];
    short2 pixeloffset[2];
    ushort2 size_sdf;
};

struct DynamicInstance {
    float projected_pos[3];
};

struct OpacityInstance {
    float fade_opacity;
};

struct DataInstance {
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity[2];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    half2 tex;

#if defined(HAS_UNIFORM_u_opacity)
    // We only need to pass `fade_opacity` separately if opacity is a
    // uniform, otherwise it's multiplied into fragment opacity, below.
    half fade_opacity;
#else
    half opacity;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const SymbolDrawableUBO* drawableVector [[buffer(idSymbolDrawableUBO)]],
                                uint instanceID [[ instance_id ]],
                                device const SymbolInstance* symbolInstances [[buffer(symbolUBOCount + 2)]],
                                device const DynamicInstance* dynamicInstances [[buffer(symbolUBOCount + 3)]],
                                device const OpacityInstance* opacityInstances [[buffer(symbolUBOCount + 4)]],
                                device const DataInstance* dataInstances [[buffer(symbolUBOCount + 5)]]) {

#if defined(HAS_UNIFORM_u_sorted_instance)
    const uint instance = instanceID;
#else
    const uint instance = vertx.sorted_instance;
#endif

    device const SymbolDrawableUBO& drawable = drawableVector[uboIndex];
    device const SymbolInstance& symbol = symbolInstances[instance];
    device const DynamicInstance& dynamic = dynamicInstances[instance];
    device const OpacityInstance& opacity = opacityInstances[instance];
    device const DataInstance& data = dataInstances[instance];

    const float2 raw_fade_opacity = unpack_opacity(opacity.fade_opacity);
    const float fade_change = raw_fade_opacity[1] > 0.5 ? paintParams.symbol_fade_change : -paintParams.symbol_fade_change;
    const float fade_opacity = max(0.0, min(1.0, raw_fade_opacity[0] + fade_change));

#if defined(HAS_UNIFORM_u_opacity)
    const half fo = half(fade_opacity);
#else
    const half fo = half(unpack_mix_float(data.opacity, drawable.opacity_t) * fade_opacity);
#endif

    // This will check to see if the opacity is zero and push the triangle offscreen if it is
    // so the GPU will cull the vertex and never send it to the fragment shader
    if (fo == 0.0) {
            return {
                .position     = float4(c_offscreen_degenerate_triangle_location,
                                                   c_offscreen_degenerate_triangle_location,
                                                   c_offscreen_degenerate_triangle_location, 1.0),
            };
        }

    const float2 a_pos = float2(symbol.pos_scale[0]);
    const float2 a_offset = float2(select(symbol.offset_tltr[uint(vertx.pos.x)], symbol.offset_blbr[uint(vertx.pos.x)], uint(vertx.pos.y)));

    const float2 a_tex = float2(symbol.texture_rect[0]) + vertx.pos * float2(symbol.texture_rect[1]);
    const float2 a_size = float2(symbol.size_sdf);

    const float a_size_min = floor(a_size[0] * 0.5);
    const float2 a_pxoffset = float2(symbol.pixeloffset[0]) + vertx.pos * float2(symbol.pixeloffset[1] - symbol.pixeloffset[0]);
    const float2 a_minFontScale = float2(symbol.pos_scale[1]) / 256.0;

    const float segment_angle = -dynamic.projected_pos[2];

    float size;
    if (!drawable.is_size_zoom_constant && !drawable.is_size_feature_constant) {
        size = mix(a_size_min, a_size[1], drawable.size_t) / 128.0;
    } else if (drawable.is_size_zoom_constant && !drawable.is_size_feature_constant) {
        size = a_size_min / 128.0;
    } else {
        size = drawable.size;
    }

    const float4 projectedPoint = drawable.matrix * float4(a_pos, 0, 1);
    const float camera_to_anchor_distance = projectedPoint.w;
    // See comments in symbol_sdf.vertex
    const float distance_ratio = drawable.pitch_with_map ?
        camera_to_anchor_distance / paintParams.camera_to_center_distance :
        paintParams.camera_to_center_distance / camera_to_anchor_distance;
    const float perspective_ratio = clamp(
            0.5 + 0.5 * distance_ratio,
            0.0, // Prevents oversized near-field symbols in pitched/overzoomed tiles
            4.0);

    if (!drawable.is_offset) {
        size *= perspective_ratio;
    }

    const float fontScale = drawable.is_text_prop ? size / 24.0 : size;

    float symbol_rotation = 0.0;
    if (drawable.rotate_symbol) {
        // See comments in symbol_sdf.vertex
        const float4 offsetProjectedPoint = drawable.matrix * float4(a_pos + float2(1, 0), 0, 1);

        const float2 a = projectedPoint.xy / projectedPoint.w;
        const float2 b = offsetProjectedPoint.xy / offsetProjectedPoint.w;
        symbol_rotation = atan2((b.y - a.y) / paintParams.aspect_ratio, b.x - a.x);
    }

    const float angle_sin = sin(segment_angle + symbol_rotation);
    const float angle_cos = cos(segment_angle + symbol_rotation);
    const float2x2 rotation_matrix = float2x2(angle_cos, -1.0 * angle_sin, angle_sin, angle_cos);

    const float4 projected_pos = drawable.label_plane_matrix * float4(dynamic.projected_pos[0], dynamic.projected_pos[1], 0.0, 1.0);
    const float2 pos0 = projected_pos.xy / projected_pos.w;
    const float2 posOffset = a_offset * max(a_minFontScale, fontScale) / 32.0 + a_pxoffset / 16.0;
    const float4 position = drawable.coord_matrix * float4(pos0 + rotation_matrix * posOffset, 0.0, 1.0);

    return {
        .position     = position,
        .tex          = half2(a_tex / drawable.texsize),
#if defined(HAS_UNIFORM_u_opacity)
        .fade_opacity = fo,
#else
        .opacity      = fo,
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const SymbolTilePropsUBO* tilePropsVector [[buffer(idSymbolTilePropsUBO)]],
                            device const SymbolEvaluatedPropsUBO& props [[buffer(idSymbolEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const SymbolTilePropsUBO& tileProps = tilePropsVector[uboIndex];

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = (tileProps.is_text ? props.text_opacity : props.icon_opacity) * in.fade_opacity;
#else
    const float opacity = in.opacity; // fade_opacity is baked in for this case
#endif

    return half4(image.sample(image_sampler, float2(in.tex)) * opacity);
}
