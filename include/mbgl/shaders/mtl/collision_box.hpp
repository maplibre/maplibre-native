// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/collision_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Metal> {
    static const ReflectionData reflectionData;
    static constexpr const char* sourceData = R"(struct VertexStage {
    short2 pos [[attribute(0)]];
    short2 anchor_pos [[attribute(1)]];
    short2 extrude [[attribute(2)]];
    uchar2 placed [[attribute(3)]];
    float2 shift [[attribute(4)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float placed;
    float notUsed;
};

struct alignas(16) CollisionUBO {
    float4x4 matrix;
    float2 extrude_scale;
    float camera_to_center_distance;
    float pad1;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const CollisionUBO& drawable [[buffer(5)]]) {

    float4 projectedPoint = drawable.matrix * float4(float2(vertx.anchor_pos), 0, 1);
    float camera_to_anchor_distance = projectedPoint.w;
    float collision_perspective_ratio = clamp(
        0.5 + 0.5 * (drawable.camera_to_center_distance / camera_to_anchor_distance),
        0.0, // Prevents oversized near-field boxes in pitched/overzoomed tiles
        4.0);

    float4 position = drawable.matrix * float4(float2(vertx.pos), 0.0, 1.0);
    position.xy += (float2(vertx.extrude) + vertx.shift) * drawable.extrude_scale * position.w * collision_perspective_ratio;

    float placed = float(vertx.placed.x);
    float notUsed = float(vertx.placed.y);

    return {
        .position       = position,
        .placed         = placed,
        .notUsed        = notUsed,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const CollisionUBO& drawable [[buffer(5)]]) {

    float alpha = 0.5;

    // Red = collision, hide label
    float4 color = float4(1.0, 0.0, 0.0, 1.0) * alpha;

    // Blue = no collision, label is showing
    if (in.placed > 0.5) {
        color = float4(0.0, 0.0, 1.0, 0.5) * alpha;
    }

    if (in.notUsed > 0.5) {
        // This box not used, fade it out
        color *= 0.1;
    }

    return half4(color);
}
)";
    static std::string source() {
        using Ty = ShaderSource<BuiltIn::CollisionBoxShader, gfx::Backend::Type::Metal>;
        return Ty::sourceData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
