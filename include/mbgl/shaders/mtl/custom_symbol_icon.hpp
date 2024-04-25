#pragma once

#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/custom_drawable_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "CustomSymbolIconShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<UniformBlockInfo, 2> uniforms;
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(
struct alignas(16) CustomSymbolIconDrawableUBO {
    float4x4 matrix;
};

struct alignas(16) CustomSymbolIconParametersUBO {
    float2 extrude_scale;
    float2 anchor;
    float angle_degrees;
    int scale_with_map;
    int pitch_with_map;
    float camera_to_center_distance;
    float aspect_ratio;
    float pad0, pad1, pad3;
};

struct VertexStage {
    float2 a_pos [[attribute(3)]];
    float2 a_tex [[attribute(4)]];
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
                                device const CustomSymbolIconDrawableUBO& drawable [[buffer(1)]],
                                device const CustomSymbolIconParametersUBO& parameters [[buffer(2)]]) {

    const float2 extrude = glMod(float2(vertx.a_pos), 2.0) * 2.0 - 1.0;
    const float2 anchor = (parameters.anchor - float2(0.5, 0.5)) * 2.0;
    const float2 center = floor(float2(vertx.a_pos) * 0.5);
    const float angle = radians(-parameters.angle_degrees);
    float2 corner = extrude - anchor;

    float4 position;
    if (parameters.pitch_with_map) {
        if (parameters.scale_with_map) {
            corner *= parameters.extrude_scale;
        } else {
            float4 projected_center = drawable.matrix * float4(center, 0, 1);
            corner *= parameters.extrude_scale * (projected_center.w / parameters.camera_to_center_distance);
        }
        corner = center + rotateVec2(corner, angle);
        position = drawable.matrix * float4(corner, 0, 1);
    } else {
        position = drawable.matrix * float4(center, 0, 1);
        const float factor = parameters.scale_with_map ? parameters.camera_to_center_distance : position.w;
        position.xy += ellipseRotateVec2(corner * parameters.extrude_scale * factor, angle, parameters.aspect_ratio);
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
)";
};

} // namespace shaders
} // namespace mbgl
