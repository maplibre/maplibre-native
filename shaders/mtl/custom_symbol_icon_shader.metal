

struct VertexStage {
    float2 a_pos [[attribute(0)]];
    float2 a_tex [[attribute(1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    half2 tex;
};

float2 rotateVec2(float2 v, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    return float2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

float2 ellipseRotateVec2(float2 v, float angle, float radiusRatio /* A/B */) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    float invRatio = 1.0 / radiusRatio;
    return float2(v.x * cosA - radiusRatio * v.y * sinA, invRatio * v.x * sinA + v.y * cosA);
}

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const CustomSymbolIconDrawableUBO& drawable [[buffer(idCustomSymbolDrawableUBO)]]) {

    const float2 extrude = glMod(float2(vertx.a_pos), 2.0) * 2.0 - 1.0;
    const float2 anchor = (drawable.anchor - float2(0.5, 0.5)) * 2.0;
    const float2 center = floor(float2(vertx.a_pos) * 0.5);
    const float angle = radians(-drawable.angle_degrees);
    float2 corner = extrude - anchor;

    float4 position;
    if (drawable.pitch_with_map) {
        if (drawable.scale_with_map) {
            corner *= drawable.extrude_scale;
        } else {
            float4 projected_center = drawable.matrix * float4(center, 0, 1);
            corner *= drawable.extrude_scale * (projected_center.w / drawable.camera_to_center_distance);
        }
        corner = center + rotateVec2(corner, angle);
        position = drawable.matrix * float4(corner, 0, 1);
    } else {
        position = drawable.matrix * float4(center, 0, 1);
        const float factor = drawable.scale_with_map ? drawable.camera_to_center_distance : position.w;
        position.xy += ellipseRotateVec2(corner * drawable.extrude_scale * factor, angle, drawable.aspect_ratio);
    }

    return {
        .position   = position,
        .tex        = half2(vertx.a_tex)
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    return half4(image.sample(image_sampler, float2(in.tex)));
}
