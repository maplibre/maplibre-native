#pragma once

#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static constexpr AttributeInfo attributes[] = {
        {0, gfx::AttributeDataType::Short2, 1, "a_pos"},
        {1, gfx::AttributeDataType::Short4, 1, "a_normal_ed"},
        {2, gfx::AttributeDataType::Float4, 1, "a_color"},
        {3, gfx::AttributeDataType::Float, 1, "a_base"},
        {4, gfx::AttributeDataType::Float, 1, "a_height"},
    };
    static constexpr UniformBlockInfo uniforms[] = {
        MLN_MTL_UNIFORM_BLOCK(5, true, false, FillExtrusionDrawableUBO),
        MLN_MTL_UNIFORM_BLOCK(6, true, false, FillExtrusionDrawablePropsUBO),
        MLN_MTL_UNIFORM_BLOCK(7, true, false, FillExtrusionInterpolateUBO),
        MLN_MTL_UNIFORM_BLOCK(8, true, false, FillExtrusionPermutationUBO),
        MLN_MTL_UNIFORM_BLOCK(9, true, false, ExpressionInputsUBO),
    };
    static constexpr TextureInfo textures[] = {};

    static constexpr auto source = R"(
struct alignas(16) FillExtrusionInterpolateUBO {
    /*  0 */ float base_t;
    /*  4 */ float height_t;
    /*  8 */ float color_t;
    /* 12 */ float pattern_from_t;
    /* 16 */ float pattern_to_t;
    /* 20 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(FillExtrusionInterpolateUBO) == 2 * 16, "unexpected padding");

struct alignas(16) FillExtrusionDrawableUBO {
    /*   0 */ float4x4 matrix;
    /*  64 */ float4 scale;
    /*  80 */ float2 texsize;
    /*  88 */ float2 pixel_coord_upper;
    /*  96 */ float2 pixel_coord_lower;
    /* 104 */ float height_factor;
    /* 108 */ float pad;
    /* 112 */
};
static_assert(sizeof(FillExtrusionDrawableUBO) == 7 * 16, "unexpected padding");

struct alignas(16) FillExtrusionDrawablePropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float4 light_color_pad;
    /* 32 */ float4 light_position_base;
    /* 48 */ float height;
    /* 52 */ float light_intensity;
    /* 56 */ float vertical_gradient;
    /* 60 */ float opacity;
    /* 64 */ float fade;
    /* 68 */ float pad2, pad3, pad4;
    /* 80 */
};
static_assert(sizeof(FillExtrusionDrawablePropsUBO) == 5 * 16, "unexpected padding");

struct alignas(16) FillExtrusionPermutationUBO {
    /*  0 */ Attribute color;
    /*  8 */ Attribute base;
    /* 16 */ Attribute height;
    /* 24 */ bool overdrawInspector;
    /* 25 */ uint8_t pad1, pad2, pad3;
    /* 28 */ float pad4;
    /* 32 */
};
static_assert(sizeof(FillExtrusionPermutationUBO) == 2 * 16, "unexpected padding");

struct VertexStage {
    short2 pos [[attribute(0)]];
    short4 normal_ed [[attribute(1)]];
    float4 color [[attribute(2)]];
    float base [[attribute(3)]];
    float height [[attribute(4)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    half4 color;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillExtrusionDrawableUBO& fill [[buffer(5)]],
                                device const FillExtrusionDrawablePropsUBO& props [[buffer(6)]],
                                device const FillExtrusionInterpolateUBO& interp [[buffer(7)]],
                                device const FillExtrusionPermutationUBO& permutation [[buffer(8)]],
                                device const ExpressionInputsUBO& expr [[buffer(9)]]) {

    const float u_base = props.light_position_base.w;
    const auto color0 =     colorFor(permutation.color,  props.color,  vertx.color,  interp.color_t,  expr);
    const auto base   = max(valueFor(permutation.base,   u_base,       vertx.base,   interp.base_t,   expr), 0.0);
    const auto height = max(valueFor(permutation.height, props.height, vertx.height, interp.height_t, expr), 0.0);

    const float3 normal = float3(vertx.normal_ed.xyz);
    const float t = fmod(normal.x, 2.0);
    const float z = (t > 0.0) ? height : base;
    const float4 position = fill.matrix * float4(float2(vertx.pos), z, 1);

    if (permutation.overdrawInspector) {
        return {
            .position = position,
            .color    = half4(1.0),
        };
    }

    // Relative luminance (how dark/bright is the surface color?)
    const float colorvalue = color0.r * 0.2126 + color0.g * 0.7152 + color0.b * 0.0722;

    // Add slight ambient lighting so no extrusions are totally black
    const float4 ambientlight = float4(0.03, 0.03, 0.03, 1.0);

    float4 color = float4(0.0, 0.0, 0.0, 1.0) + ambientlight;

    // Calculate cos(theta), where theta is the angle between surface normal and diffuse light ray
    float directional = clamp(dot(normal / 16384.0, props.light_position_base.xyz), 0.0, 1.0);

    // Adjust directional so that the range of values for highlight/shading is
    // narrower with lower light intensity and with lighter/brighter surface colors
    directional = mix((1.0 - props.light_intensity), max((1.0 - colorvalue + props.light_intensity), 1.0), directional);

    // Add gradient along z axis of side surfaces
    if (normal.y != 0.0) {
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
    // TODO: rewrite with vector operations
    const float3 light_color = props.light_color_pad.rgb;
    color.r += clamp(color.r * directional * light_color.r, mix(0.0, 0.3, 1.0 - light_color.r), 1.0);
    color.g += clamp(color.g * directional * light_color.g, mix(0.0, 0.3, 1.0 - light_color.g), 1.0);
    color.b += clamp(color.b * directional * light_color.b, mix(0.0, 0.3, 1.0 - light_color.b), 1.0);

    return {
        .position = position,
        .color    = half4(color * props.opacity),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]]) {
    return in.color;
}
)";
};

} // namespace shaders
} // namespace mbgl
