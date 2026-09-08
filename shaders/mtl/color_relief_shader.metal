struct VertexStage {
    short2 pos [[attribute(0)]];
    short2 texture_pos [[attribute(1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const ColorReliefDrawableUBO* drawableVector [[buffer(idColorReliefDrawableUBO)]],
                                device const ColorReliefTilePropsUBO* tilePropsVector [[buffer(idColorReliefTilePropsUBO)]]) {

    device const ColorReliefDrawableUBO& drawable = drawableVector[uboIndex];
    device const ColorReliefTilePropsUBO& tileProps = tilePropsVector[uboIndex];

    const float4 position = drawable.matrix * float4(float2(vertx.pos), 0, 1);

    // Calculate texture coordinate
    float2 epsilon = 1.0 / tileProps.dimension;
    float scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    float2 pos = (float2(vertx.pos) / 8192.0) * scale + epsilon;

    // Handle poles (use vertx.pos, not texture_pos, to match GLSL a_pos)
    if (float(vertx.pos.y) < -32767.5) pos.y = 0.0;
    if (float(vertx.pos.y) > 32766.5) pos.y = 1.0;

    return {
        .position    = position,
        .pos         = pos,
    };
}

// Function to convert terrain-RGB encoded elevation value to meters
float getElevation(float2 coord, texture2d<float, access::sample> image, sampler image_sampler, float4 unpack) {
    float4 data = image.sample(image_sampler, coord) * 255.0;
    data.a = -1.0;
    return dot(data, unpack);
}

// Function to get the elevation value at a specific color ramp stop
float getElevationStop(int stop, int color_ramp_size, texture2d<float, access::sample> elevationStops, sampler elevation_sampler) {
    // Elevation stops are plain float values, stored in the R channel
    float x = (float(stop) + 0.5) / float(color_ramp_size);
    return elevationStops.sample(elevation_sampler, float2(x, 0.5)).r;
}

// Function to get the color value at a specific color ramp stop
float4 getColorStop(int stop, int color_ramp_size, texture2d<float, access::sample> colorStops, sampler color_sampler) {
    float x = (float(stop) + 0.5) / float(color_ramp_size);
    return colorStops.sample(color_sampler, float2(x, 0.5));
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const ColorReliefTilePropsUBO* tilePropsVector [[buffer(idColorReliefTilePropsUBO)]],
                            device const ColorReliefEvaluatedPropsUBO& props [[buffer(idColorReliefEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            texture2d<float, access::sample> elevationStops [[texture(1)]],
                            texture2d<float, access::sample> colorStops [[texture(2)]],
                            sampler image_sampler [[sampler(0)]],
                            sampler elevation_stops_sampler [[sampler(1)]],
                            sampler color_stops_sampler [[sampler(2)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const ColorReliefTilePropsUBO& tileProps = tilePropsVector[uboIndex];

    // 1. Get elevation at this pixel from DEM
    float el = getElevation(in.pos, image, image_sampler, tileProps.unpack);

    // 2. Binary search to find surrounding elevation stops (l and r indices)
    int r = tileProps.color_ramp_size - 1;
    int l = 0;

    // Perform binary search
    while (r - l > 1) {
        int m = (r + l) / 2;
        float el_m = getElevationStop(m, tileProps.color_ramp_size, elevationStops, elevation_stops_sampler);

        if (el < el_m) {
            r = m;
        } else {
            l = m;
        }
    }

    // 3. Get the elevation values and colors at the stops
    float el_l = getElevationStop(l, tileProps.color_ramp_size, elevationStops, elevation_stops_sampler);
    float el_r = getElevationStop(r, tileProps.color_ramp_size, elevationStops, elevation_stops_sampler);

    float4 color_l = getColorStop(l, tileProps.color_ramp_size, colorStops, color_stops_sampler);
    float4 color_r = getColorStop(r, tileProps.color_ramp_size, colorStops, color_stops_sampler);

    // 4. Interpolate color based on elevation
    // Guard against division by zero when el_r == el_l
    float denom = el_r - el_l;
    float t = (abs(denom) < 0.0001) ? 0.0 : clamp((el - el_l) / denom, 0.0, 1.0);

    thread half4 fragColor;
    float4 color = mix(color_l, color_r, t);
    color *= props.opacity;
    fragColor = half4(color);
    return fragColor;
}
