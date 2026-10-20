// Metal source for the `gltf` plugin layer: a lit mesh drawn at every point of the layer's source.
//
// The mesh is stored once per point with its model-space offset kept apart from the point's tile
// coordinate, so heading, size, altitude and opacity are evaluated per frame as paint properties rather
// than baked into the tile at layout time. Size is either in meters (`model-scale`) or held constant on
// screen (`model-screen-scale`, in points per mesh unit), the way the system maps keep their puck.
#pragma once

inline constexpr char gltfMetalSource[] = R"SHADER(
#include <metal_stdlib>
using namespace metal;

struct alignas(16) GltfUBO {
    float4x4 matrix;
    float4 camera;  // pixels_to_gl_units.xy, pixels_to_tile_units, camera_to_center_distance
    float4 view;    // bearing, pixel_ratio, pixels_per_meter, unused
    float heading;
    float scale;
    float screen_scale;
    float opacity;
    float4 altitude; // x only
    float4 interpolation[4];
};

struct GltfVertex {
    short2 a_position [[attribute(0)]];   // the anchor's tile coordinate, repeated for every mesh vertex
#if !MLN_PLUGIN_PROPERTY_MODEL_HEADING_IS_UNIFORM
    float2 a_heading [[attribute(1)]];
#endif
#if !MLN_PLUGIN_PROPERTY_MODEL_SCALE_IS_UNIFORM
    float2 a_scale [[attribute(2)]];
#endif
#if !MLN_PLUGIN_PROPERTY_MODEL_SCREEN_SCALE_IS_UNIFORM
    float2 a_screen_scale [[attribute(3)]];
#endif
#if !MLN_PLUGIN_PROPERTY_MODEL_OPACITY_IS_UNIFORM
    float2 a_opacity [[attribute(4)]];
#endif
#if !MLN_PLUGIN_PROPERTY_MODEL_ALTITUDE_IS_UNIFORM
    float2 a_altitude [[attribute(5)]];
#endif
    float3 a_model [[attribute(6)]];      // mesh vertex, in mesh units
    float3 a_normal [[attribute(7)]];
    float4 a_color [[attribute(8)]];      // straight alpha
};

struct GltfVaryings {
    float4 position [[position]];
    float3 normal;
    float4 color;
};

vertex GltfVaryings gltfVertex(GltfVertex in [[stage_in]],
                               constant GltfUBO& u [[buffer(MLN_PLUGIN_UNIFORM_0_BINDING)]]) {
    float heading = u.heading;
#if !MLN_PLUGIN_PROPERTY_MODEL_HEADING_IS_UNIFORM
    heading = mix(in.a_heading.x, in.a_heading.y, u.interpolation[0].x);
#endif
    float scale = u.scale;
#if !MLN_PLUGIN_PROPERTY_MODEL_SCALE_IS_UNIFORM
    scale = mix(in.a_scale.x, in.a_scale.y, u.interpolation[0].y);
#endif
    float screenScale = u.screen_scale;
#if !MLN_PLUGIN_PROPERTY_MODEL_SCREEN_SCALE_IS_UNIFORM
    screenScale = mix(in.a_screen_scale.x, in.a_screen_scale.y, u.interpolation[0].z);
#endif
    float opacity = u.opacity;
#if !MLN_PLUGIN_PROPERTY_MODEL_OPACITY_IS_UNIFORM
    opacity = mix(in.a_opacity.x, in.a_opacity.y, u.interpolation[0].w);
#endif
    float altitude = u.altitude.x;
#if !MLN_PLUGIN_PROPERTY_MODEL_ALTITUDE_IS_UNIFORM
    altitude = mix(in.a_altitude.x, in.a_altitude.y, u.interpolation[1].x);
#endif

    float2 center = float2(in.a_position);
    // The projection reads z as meters and scales it by pixels-per-meter. An older host without the
    // field leaves it at 0, which is treated as 1.
    float pixelsPerMeter = u.view.z > 0.0 ? u.view.z : 1.0;

    float tileUnitsPerMeshUnit;
    float metersPerMeshUnit;
    if (screenScale > 0.0) {
        // Points per mesh unit, corrected by the perspective ratio so the on-screen size stays put
        // when the camera pitches.
        float4 projected = u.matrix * float4(center, 0.0, 1.0);
        float pixels = screenScale * (projected.w / u.camera.w);
        tileUnitsPerMeshUnit = pixels * u.camera.z;
        metersPerMeshUnit = pixels / pixelsPerMeter;
    } else {
        tileUnitsPerMeshUnit = scale * pixelsPerMeter * u.camera.z;
        metersPerMeshUnit = scale;
    }

    float b = heading * 0.017453292519943295;
    float c = cos(b);
    float s = sin(b);
    float2 ground = float2(c * in.a_model.x + s * in.a_model.z, s * in.a_model.x - c * in.a_model.z);

    GltfVaryings out;
    out.position = u.matrix * float4(center + ground * tileUnitsPerMeshUnit,
                                     altitude + in.a_model.y * metersPerMeshUnit,
                                     1.0);
    out.normal = float3(c * in.a_normal.x + s * in.a_normal.z, s * in.a_normal.x - c * in.a_normal.z, in.a_normal.y);
    out.color = float4(in.a_color.rgb, in.a_color.a * clamp(opacity, 0.0, 1.0));
    return out;
}

fragment float4 gltfFragment(GltfVaryings in [[stage_in]]) {
    float light = 0.32 + 0.68 * max(dot(normalize(in.normal), normalize(float3(0.35, -0.45, 0.82))), 0.0);
    return float4(in.color.rgb * light * in.color.a, in.color.a);
}
)SHADER";
