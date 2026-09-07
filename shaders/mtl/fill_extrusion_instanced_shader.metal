

struct VertexStage {
    short2 pos [[attribute(0)]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color [[attribute(3)]];
#endif
#if !defined(HAS_UNIFORM_u_base)
    float2 base [[attribute(4)]];
#endif
#if !defined(HAS_UNIFORM_u_height)
    float2 height [[attribute(5)]];
#endif
};

struct OutlineInstance {
    short2 pos;
    ushort2 decimals_ed;
};

struct FragmentStage {
    float4 position [[position, invariant]];
    half4 color;
};

struct FragmentOutput {
    half4 color [[color(0)]];
    //float depth [[depth(less)]]; // Write depth value if it's less than what's already there
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillExtrusionDrawableUBO* drawableVector [[buffer(idFillExtrusionDrawableUBO)]],
                                device const FillExtrusionPropsUBO& props [[buffer(idFillExtrusionPropsUBO)]],
                                uint instanceID [[ instance_id ]],
                                device const OutlineInstance* outline [[buffer(fillExtrusionUBOCount + 1)]]) {

    bool isDiscarded = glMod(float(outline[instanceID].decimals_ed.x), 2.0) > 0.0;
    if (isDiscarded) {
        return {
            .position = float4(0.0),
            .color    = half4(0.0),
        };
    }

    device const FillExtrusionDrawableUBO& drawable = drawableVector[uboIndex];

#if defined(HAS_UNIFORM_u_base)
    const auto base   = props.light_position_base.w;
#else
    const auto base   = max(unpack_mix_float(vertx.base, drawable.base_t), 0.0);
#endif
#if defined(HAS_UNIFORM_u_height)
    const auto height = props.height;
#else
    const auto height = max(unpack_mix_float(vertx.height, drawable.height_t), 0.0);
#endif

    const float2 p1 = float2(outline[instanceID + 0].pos) + unpack_float(float(outline[instanceID + 0].decimals_ed.x / 2)) / 128.0;
    const float2 p2 = float2(outline[instanceID + 1].pos) + unpack_float(float(outline[instanceID + 1].decimals_ed.x / 2)) / 128.0;
    const float2 edgevector = normalize(p2 - p1);

    const float3 normal = float3(-edgevector.y, edgevector.x, 0.0);
    const float t = float(vertx.pos.y);
    const float z = (t != 0.0) ? height : base;     // TODO: This would come out wrong on GL for negative values, check it...

    const float4 position = drawable.matrix * float4(vertx.pos.x == 0.0 ? p1 : p2, z, 1);

#if defined(OVERDRAW_INSPECTOR)
    return {
        .position = position,
        .color    = half4(1.0),
    };
#endif

#if defined(HAS_UNIFORM_u_color)
    auto color = props.color;
#else
    auto color = unpack_mix_color(vertx.color, drawable.color_t);
#endif

    // Relative luminance (how dark/bright is the surface color?)
    const float luminance = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;

    float4 vcolor = float4(0.0, 0.0, 0.0, 1.0);

    // Add slight ambient lighting so no extrusions are totally black
    color += min(float4(0.03, 0.03, 0.03, 1.0), float4(1.0));

    // Calculate cos(theta), where theta is the angle between surface normal and diffuse light ray
    const float directionalFraction = clamp(dot(normal, props.light_position_base.xyz), 0.0, 1.0);

    // Adjust directional so that the range of values for highlight/shading is
    // narrower with lower light intensity and with lighter/brighter surface colors
    const float minDirectional = 1.0 - props.light_intensity;
    const float maxDirectional = max(1.0 - luminance + props.light_intensity, 1.0);
    float directional = mix(minDirectional, maxDirectional, directionalFraction);

    // Add gradient along z axis of side surfaces
    if (normal.z == 0.0) {
        // This avoids another branching statement, but multiplies by a constant of 0.84 if no
        // vertical gradient, and otherwise calculates the gradient based on base + height
        // TODO: If we're optimizing to the level of avoiding branches, we should pre-compute
        //       the square root when height is a uniform.
        const float fMin = mix(0.7, 0.98, 1.0 - props.light_intensity);
        const float factor = clamp((t + base) * pow(height / 150.0, 0.5), fMin, 1.0);
        directional *= (1.0 - props.vertical_gradient) + (props.vertical_gradient * factor);
    }

    // Assign final color based on surface + ambient light color, diffuse light directional,
    // and light color with lower bounds adjusted to hue of light so that shading is tinted
    // with the complementary (opposite) color to the light color
    const float3 light_color = props.light_color_pad.rgb;
    const float3 minLight = mix(0.0, 0.3, 1.0 - light_color.rgb);
    vcolor += float4(clamp(color.rgb * directional * light_color.rgb, minLight, 1.0), 0.0);

    return {
        .position = position,
        .color    = half4(vcolor * props.opacity),
    };
}

fragment FragmentOutput fragmentMain(FragmentStage in [[stage_in]]) {
    return { in.color/*, in.position.z*/ };
}
