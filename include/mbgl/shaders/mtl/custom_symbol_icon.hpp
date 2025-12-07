#pragma once

#include <mbgl/shaders/custom_drawable_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto customSymbolIconShaderPrelude = R"(

enum {
    idCustomSymbolDrawableUBO = idDrawableReservedVertexOnlyUBO,
    customSymbolUBOCount = drawableReservedUBOCount
};

struct alignas(16) CustomSymbolIconDrawableUBO {
    /*   0 */ float4x4 matrix;
    /*  64 */ float2 extrude_scale;
    /*  72 */ float2 anchor;
    /*  80 */ float angle_degrees;
    /*  84 */ uint32_t scale_with_map;
    /*  88 */ uint32_t pitch_with_map;
    /*  92 */ float camera_to_center_distance;
    /*  96 */ float aspect_ratio;
    /* 100 */ float pad1;
    /* 104 */ float pad2;
    /* 108 */ float pad3;
    /* 112 */
};
static_assert(sizeof(CustomSymbolIconDrawableUBO) == 7 * 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "CustomSymbolIconShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = customSymbolIconShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    float2 a_pos [[attribute(customSymbolUBOCount + 0)]];
    float2 a_tex [[attribute(customSymbolUBOCount + 1)]];
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

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            texture2d<float, access::sample> image [[texture(0)]],
                            sampler image_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return PrecisionFloat4(1.0);
#endif

    return PrecisionFloat4(image.sample(image_sampler, float2(in.tex)));
}
)";
};

} // namespace shaders
} // namespace mbgl
