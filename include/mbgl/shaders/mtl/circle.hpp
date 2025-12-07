#pragma once

#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto circleShaderPrelude = R"(

enum {
    idCircleDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idCircleEvaluatedPropsUBO = drawableReservedUBOCount,
    circleUBOCount
};

struct alignas(16) CircleDrawableUBO {
    /*   0 */ float4x4 matrix;
    /*  64 */ float2 extrude_scale;

    // Interpolations
    /*  72 */ float color_t;
    /*  76 */ float radius_t;
    /*  80 */ float blur_t;
    /*  84 */ float opacity_t;
    /*  88 */ float stroke_color_t;
    /*  92 */ float stroke_width_t;
    /*  96 */ float stroke_opacity_t;
    /* 100 */ float pad1;
    /* 104 */ float pad2;
    /* 108 */ float pad3;
    /* 112 */
};
static_assert(sizeof(CircleDrawableUBO) == 7 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) CircleEvaluatedPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float4 stroke_color;
    /* 32 */ float radius;
    /* 36 */ float blur;
    /* 40 */ float opacity;
    /* 44 */ float stroke_width;
    /* 48 */ float stroke_opacity;
    /* 52 */ int scale_with_map;
    /* 56 */ int pitch_with_map;
    /* 60 */ float pad1;
    /* 64 */
};
static_assert(sizeof(CircleEvaluatedPropsUBO) == 4 * 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "CircleShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = circleShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(circleUBOCount + 0)]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color [[attribute(circleUBOCount + 1)]];
#endif
#if !defined(HAS_UNIFORM_u_radius)
    float2 radius [[attribute(circleUBOCount + 2)]];
#endif
#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(circleUBOCount + 3)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(circleUBOCount + 4)]];
#endif
#if !defined(HAS_UNIFORM_u_stroke_color)
    float4 stroke_color [[attribute(circleUBOCount + 5)]];
#endif
#if !defined(HAS_UNIFORM_u_stroke_width)
    float2 stroke_width [[attribute(circleUBOCount + 6)]];
#endif
#if !defined(HAS_UNIFORM_u_stroke_opacity)
    float2 stroke_opacity [[attribute(circleUBOCount + 7)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 extrude;
    float antialiasblur;

#if !defined(HAS_UNIFORM_u_color)
    PrecisionFloat4 color;
#endif
#if !defined(HAS_UNIFORM_u_radius)
    float radius;
#endif
#if !defined(HAS_UNIFORM_u_blur)
    half blur;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
#if !defined(HAS_UNIFORM_u_stroke_color)
    PrecisionFloat4 stroke_color;
#endif
#if !defined(HAS_UNIFORM_u_stroke_width)
    half stroke_width;
#endif
#if !defined(HAS_UNIFORM_u_stroke_opacity)
    half stroke_opacity;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const CircleDrawableUBO* drawableVector [[buffer(idCircleDrawableUBO)]],
                                device const CircleEvaluatedPropsUBO& props [[buffer(idCircleEvaluatedPropsUBO)]]) {

    device const CircleDrawableUBO& drawable = drawableVector[uboIndex];

#if defined(HAS_UNIFORM_u_radius)
    const auto radius       = props.radius;
#else
    const auto radius       = unpack_mix_float(vertx.radius, drawable.radius_t);
#endif

#if defined(HAS_UNIFORM_u_stroke_width)
    const auto stroke_width = props.stroke_width;
#else
    const auto stroke_width = unpack_mix_float(vertx.stroke_width, drawable.stroke_width_t);
#endif

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
            const float4 projected_center = drawable.matrix * float4(circle_center, 0, 1);
            corner_position += scaled_extrude * (radius + stroke_width) *
                               (projected_center.w / paintParams.camera_to_center_distance);
        }

        position = drawable.matrix * float4(corner_position, 0, 1);
    } else {
        position = drawable.matrix * float4(circle_center, 0, 1);

        const float factor = props.scale_with_map ? paintParams.camera_to_center_distance : position.w;
        position.xy += scaled_extrude * (radius + stroke_width) * factor;
    }

    // This is a minimum blur distance that serves as a faux-antialiasing for
    // the circle. since blur is a ratio of the circle's size and the intent is
    // to keep the blur at roughly 1px, the two are inversely related.
    const half antialiasblur = 1.0 / DEVICE_PIXEL_RATIO / (radius + stroke_width);

    return {
        .position       = position,
        .extrude        = extrude,
        .antialiasblur  = antialiasblur,

#if !defined(HAS_UNIFORM_u_color)
        .color          = PrecisionFloat4(unpack_mix_color(vertx.color, drawable.color_t)),
#endif
#if !defined(HAS_UNIFORM_u_radius)
        .radius         = radius,
#endif
#if !defined(HAS_UNIFORM_u_blur)
        .blur           = half(unpack_mix_float(vertx.blur, drawable.blur_t)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity        = half(unpack_mix_float(vertx.opacity, drawable.opacity_t)),
#endif
#if !defined(HAS_UNIFORM_u_stroke_color)
        .stroke_color   = PrecisionFloat4(unpack_mix_color(vertx.stroke_color, drawable.stroke_color_t)),
#endif
#if !defined(HAS_UNIFORM_u_stroke_width)
        .stroke_width   = half(stroke_width),
#endif
#if !defined(HAS_UNIFORM_u_stroke_opacity)
        .stroke_opacity = half(unpack_mix_float(vertx.stroke_opacity, drawable.stroke_opacity_t)),
#endif
    };
}

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const CircleEvaluatedPropsUBO& props [[buffer(idCircleEvaluatedPropsUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return PrecisionFloat4(1.0, 1.0, 1.0, 1.0);
#endif

#if defined(HAS_UNIFORM_u_color)
    const PrecisionFloat4 color = PrecisionFloat4(props.color);
#else
    const PrecisionFloat4 color = in.color;
#endif
#if defined(HAS_UNIFORM_u_radius)
    const float radius = props.radius;
#else
    const float radius = in.radius;
#endif
#if defined(HAS_UNIFORM_u_blur)
    const float blur = props.blur;
#else
    const float blur = in.blur;
#endif
#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = in.opacity;
#endif
#if defined(HAS_UNIFORM_u_stroke_color)
    const PrecisionFloat4 stroke_color = PrecisionFloat4(props.stroke_color);
#else
    const PrecisionFloat4 stroke_color = in.stroke_color;
#endif
#if defined(HAS_UNIFORM_u_stroke_width)
    const float stroke_width = props.stroke_width;
#else
    const float stroke_width = in.stroke_width;
#endif
#if defined(HAS_UNIFORM_u_stroke_opacity)
    const float stroke_opacity = props.stroke_opacity;
#else
    const float stroke_opacity = in.stroke_opacity;
#endif

    const float extrude_length = length(in.extrude);
    const float antialiased_blur = -max(blur, in.antialiasblur);
    const float opacity_t = smoothstep(0.0, antialiased_blur, extrude_length - 1.0);
    const float color_t = (stroke_width < 0.01) ? 0.0 :
        smoothstep(antialiased_blur, 0.0, extrude_length - radius / (radius + stroke_width));

    return PrecisionFloat4(opacity_t * mix(color * opacity, stroke_color * stroke_opacity, color_t));
}
)";
};

} // namespace shaders
} // namespace mbgl
