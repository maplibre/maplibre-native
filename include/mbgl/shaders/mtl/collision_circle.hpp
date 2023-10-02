#pragma once

#include <mbgl/shaders/collision_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CollisionCircleShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "CollisionCircleShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 4> attributes;
    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(0)]];
    short2 anchor_pos [[attribute(1)]];
    short2 extrude [[attribute(2)]];
    uchar2 placed [[attribute(3)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float placed;
    float notUsed;
    float radius;
    float2 extrude;
    float2 extrude_scale;
};

struct alignas(16) CollisionCircleUBO {
    float4x4 matrix;
    float2 extrude_scale;
    float camera_to_center_distance;
    float overscale_factor;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const CollisionCircleUBO& drawable [[buffer(4)]]) {

    float4 projectedPoint = drawable.matrix * float4(float2(vertx.anchor_pos), 0, 1);
    float camera_to_anchor_distance = projectedPoint.w;
    float collision_perspective_ratio = clamp(
        0.5 + 0.5 * (drawable.camera_to_center_distance / camera_to_anchor_distance),
        0.0, // Prevents oversized near-field circles in pitched/overzoomed tiles
        4.0);

    float4 position = drawable.matrix * float4(float2(vertx.pos), 0.0, 1.0);

    float padding_factor = 1.2; // Pad the vertices slightly to make room for anti-alias blur
    position.xy += float2(vertx.extrude) * drawable.extrude_scale * padding_factor * position.w * collision_perspective_ratio;

    float placed = float(vertx.placed.x);
    float notUsed = float(vertx.placed.y);
    float radius = abs(float(vertx.extrude.y)); // We don't pitch the circles, so both units of the extrusion vector are equal in magnitude to the radius

    float2 extrude = float2(vertx.extrude) * padding_factor;
    float2 extrude_scale = drawable.extrude_scale * drawable.camera_to_center_distance * collision_perspective_ratio;

    return {
        .position       = position,
        .placed         = placed,
        .notUsed        = notUsed,
        .radius         = radius,
        .extrude        = extrude,
        .extrude_scale  = extrude_scale,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const CollisionCircleUBO& drawable [[buffer(4)]]) {

    float alpha = 0.5;

    // Red = collision, hide label
    float4 color = float4(1.0, 0.0, 0.0, 1.0) * alpha;

    // Blue = no collision, label is showing
    if (in.placed > 0.5) {
        color = float4(0.0, 0.0, 1.0, 0.5) * alpha;
    }

    if (in.notUsed > 0.5) {
        // This box not used, fade it out
        color *= 0.2;
    }

    float extrude_scale_length = length(in.extrude_scale);
    float extrude_length = length(in.extrude) * extrude_scale_length;
    float stroke_width = 15.0 * extrude_scale_length / drawable.overscale_factor;
    float radius = in.radius * extrude_scale_length;

    float distance_to_edge = abs(extrude_length - radius);
    float opacity_t = smoothstep(-stroke_width, 0.0, -distance_to_edge);

    return half4(opacity_t * color);
}
)";
};

} // namespace shaders
} // namespace mbgl
