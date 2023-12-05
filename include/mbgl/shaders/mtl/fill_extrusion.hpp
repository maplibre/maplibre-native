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
    static constexpr auto hasPermutations = false;

    static const std::array<AttributeInfo, 5> attributes;
    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<TextureInfo, 0> textures;

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
    /* 24 */ Attribute pattern_from;
    /* 32 */ Attribute pattern_to;
    /* 40 */ bool overdrawInspector;
    /* 41 */ uint8_t pad1, pad2, pad3;
    /* 44 */ float pad4;
    /* 48 */
};
static_assert(sizeof(FillExtrusionPermutationUBO) == 3 * 16, "unexpected padding");

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

struct FragmentOutput {
    half4 color [[color(0)]];
    //float depth [[depth(less)]]; // Write depth value if it's less than what's already there
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillExtrusionDrawableUBO& fill [[buffer(5)]],
                                device const FillExtrusionDrawablePropsUBO& props [[buffer(6)]],
                                device const FillExtrusionInterpolateUBO& interp [[buffer(7)]],
                                device const FillExtrusionPermutationUBO& permutation [[buffer(8)]],
                                device const ExpressionInputsUBO& expr [[buffer(9)]]) {

    const float u_base = props.light_position_base.w;
    const auto base   = max(valueFor(permutation.base,   u_base,       vertx.base,   interp.base_t,   expr), 0.0);
    const auto height = max(valueFor(permutation.height, props.height, vertx.height, interp.height_t, expr), 0.0);

    const float3 normal = float3(vertx.normal_ed.xyz);
    const float t = glMod(normal.x, 2.0);
    const float z = (t != 0.0) ? height : base;     // TODO: This would come out wrong on GL for negative values, check it...
    const float4 position = fill.matrix * float4(float2(vertx.pos), z, 1);

    if (permutation.overdrawInspector) {
        return {
            .position = position,
            .color    = half4(1.0),
        };
    }

    auto color = colorFor(permutation.color, props.color, vertx.color, interp.color_t, expr);

    // Relative luminance (how dark/bright is the surface color?)
    const float luminance = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;

    float4 vcolor = float4(0.0, 0.0, 0.0, 1.0);

    // Add slight ambient lighting so no extrusions are totally black
    color += min(float4(0.03, 0.03, 0.03, 1.0), float4(1.0));

    // Calculate cos(theta), where theta is the angle between surface normal and diffuse light ray
    const float directionalFraction = clamp(dot(normal / 16384.0, props.light_position_base.xyz), 0.0, 1.0);

    // Adjust directional so that the range of values for highlight/shading is
    // narrower with lower light intensity and with lighter/brighter surface colors
    const float minDirectional = 1.0 - props.light_intensity;
    const float maxDirectional = max(1.0 - luminance + props.light_intensity, 1.0);
    float directional = mix(minDirectional, maxDirectional, directionalFraction);

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
    const float3 light_color = props.light_color_pad.rgb;
    const float3 minLight = mix(0.0, 0.3, 1.0 - light_color.rgb);
    vcolor += float4(clamp(color.rgb * directional * light_color.rgb, minLight, 1.0), 0.0);

    return {
        .position = position,
        .color    = half4(vcolor * props.opacity),
    };
}

fragment FragmentOutput fragmentMain(FragmentStage in [[stage_in]]) {
    return { in.color/*, in.position.z*/ };
}
)";
};

} // namespace shaders
} // namespace mbgl
