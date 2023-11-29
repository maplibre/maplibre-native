#pragma once

#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "CircleShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = false;

    static const std::array<AttributeInfo, 8> attributes;
    static const std::array<UniformBlockInfo, 6> uniforms;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(0)]];
    float4 color [[attribute(1)]];
    float2 radius [[attribute(2)]];
    float2 blur [[attribute(3)]];
    float2 opacity [[attribute(4)]];
    float4 stroke_color [[attribute(5)]];
    float2 stroke_width [[attribute(6)]];
    float2 stroke_opacity [[attribute(7)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 color;
    float radius;
    half blur;
    half opacity;
    float4 stroke_color;
    float stroke_width;
    half stroke_opacity;

    float2 extrude;
    half antialiasblur;
};

struct alignas(16) CircleDrawableUBO {
    float4x4 matrix;
    float2 extrude_scale;
    float2 padding;
};
struct alignas(16) CirclePaintParamsUBO {
    float camera_to_center_distance;
    float device_pixel_ratio;
    float pad1,pad2;
};
struct alignas(16) CircleEvaluatedPropsUBO {
    float4 color;
    float4 stroke_color;
    float radius;
    float blur;
    float opacity;
    float stroke_width;
    float stroke_opacity;
    int scale_with_map;
    int pitch_with_map;
    float padding;
};

struct alignas(16) CircleInterpolateUBO {
    float color_t;
    float radius_t;
    float blur_t;
    float opacity_t;
    float stroke_color_t;
    float stroke_width_t;
    float stroke_opacity_t;
    float pad1_;
};

struct alignas(16) CirclePermutationUBO {
    Attribute color;
    Attribute radius;
    Attribute blur;
    Attribute opacity;
    Attribute stroke_color;
    Attribute stroke_width;
    Attribute stroke_opacity;
    bool overdrawInspector;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const CircleDrawableUBO& drawable [[buffer(8)]],
                                device const CirclePaintParamsUBO& params [[buffer(9)]],
                                device const CircleEvaluatedPropsUBO& props [[buffer(10)]],
                                device const CircleInterpolateUBO& interp [[buffer(11)]],
                                device const CirclePermutationUBO& permutation [[buffer(12)]],
                                device const ExpressionInputsUBO& expr [[buffer(13)]]) {

    const auto color          = colorFor(permutation.color,          props.color,          vertx.color,          interp.color_t,          expr);
    const auto radius         = valueFor(permutation.radius,         props.radius,         vertx.radius,         interp.radius_t,         expr);
    const auto blur           = valueFor(permutation.blur,           props.blur,           vertx.blur,           interp.blur_t,           expr);
    const auto opacity        = valueFor(permutation.opacity,        props.opacity,        vertx.opacity,        interp.opacity_t,        expr);
    const auto stroke_color   = colorFor(permutation.stroke_color,   props.stroke_color,   vertx.stroke_color,   interp.stroke_color_t,   expr);
    const auto stroke_width   = valueFor(permutation.stroke_width,   props.stroke_width,   vertx.stroke_width,   interp.stroke_width_t,   expr);
    const auto stroke_opacity = valueFor(permutation.stroke_opacity, props.stroke_opacity, vertx.stroke_opacity, interp.stroke_opacity_t, expr);

    // unencode the extrusion vector that we snuck into the a_pos vector
    const float2 extrude = glMod(float2(vertx.position), 2.0) * 2.0 - 1.0;
    const float2 scaled_extrude = extrude * drawable.extrude_scale;

    // multiply a_pos by 0.5, since we had it * 2 in order to sneak in extrusion data
    const float2 circle_center = floor(float2(vertx.position) * 0.5);

    float4 position;
    if (props.pitch_with_map) {
        float2 corner_position = circle_center;
        if (props.scale_with_map) {
            corner_position += scaled_extrude * (radius + stroke_width);
        } else {
            // Pitching the circle with the map effectively scales it with the map
            // To counteract the effect for pitch-scale: viewport, we rescale the
            // whole circle based on the pitch scaling effect at its central point
            float4 projected_center = drawable.matrix * float4(circle_center, 0, 1);
            corner_position += scaled_extrude * (radius + stroke_width) *
                               (projected_center.w / params.camera_to_center_distance);
        }

        position = drawable.matrix * float4(corner_position, 0, 1);
    } else {
        position = drawable.matrix * float4(circle_center, 0, 1);

        const float factor = props.scale_with_map ? params.camera_to_center_distance : position.w;
        position.xy += scaled_extrude * (radius + stroke_width) * factor;
    }

    // This is a minimum blur distance that serves as a faux-antialiasing for
    // the circle. since blur is a ratio of the circle's size and the intent is
    // to keep the blur at roughly 1px, the two are inversely related.
    const half antialiasblur = 1.0 / params.device_pixel_ratio / (radius + stroke_width);

    return {
        .position       = position,
        .color          = color,
        .radius         = radius,
        .blur           = half(blur),
        .opacity        = half(opacity),
        .stroke_color   = stroke_color,
        .stroke_width   = stroke_width,
        .stroke_opacity = half(stroke_opacity),
        .extrude        = extrude,
        .antialiasblur  = antialiasblur,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const CirclePaintParamsUBO& params [[buffer(9)]],
                            device const CircleEvaluatedPropsUBO& props [[buffer(10)]],
                            device const CirclePermutationUBO& permutation [[buffer(12)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    const float extrude_length = length(in.extrude);
    const float antialiased_blur = -max(in.blur, in.antialiasblur);
    const float opacity_t = smoothstep(0.0, antialiased_blur, extrude_length - 1.0);
    const float color_t = (in.stroke_width < 0.01) ? 0.0 :
        smoothstep(antialiased_blur, 0.0, extrude_length - in.radius / (in.radius + in.stroke_width));

    return half4(opacity_t * mix(in.color * in.opacity, in.stroke_color * in.stroke_opacity, color_t));
}
)";
};

} // namespace shaders
} // namespace mbgl
