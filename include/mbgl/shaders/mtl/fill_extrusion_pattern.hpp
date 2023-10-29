#pragma once

#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionPatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 6> attributes;
    static const std::array<UniformBlockInfo, 6> uniforms;
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(
struct alignas(16) FillExtrusionDrawableTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */
};

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
    float base [[attribute(2)]];
    float height [[attribute(3)]];
    ushort4 pattern_from [[attribute(4)]];
    ushort4 pattern_to [[attribute(5)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 lighting;
    float4 pattern_from;
    float4 pattern_to;
    float2 pos_a;
    float2 pos_b;
};

struct FragmentOutput {
    half4 color [[color(0)]];
    //float depth [[depth(less)]]; // Write depth value if it's less than what's already there
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillExtrusionDrawableUBO& fill [[buffer(6)]],
                                device const FillExtrusionDrawablePropsUBO& props [[buffer(7)]],
                                device const FillExtrusionDrawableTilePropsUBO& tileProps [[buffer(8)]],
                                device const FillExtrusionInterpolateUBO& interp [[buffer(9)]],
                                device const FillExtrusionPermutationUBO& permutation [[buffer(10)]],
                                device const ExpressionInputsUBO& expr [[buffer(11)]]) {

    const float u_base = props.light_position_base.w;
    const auto base   = max(valueFor(permutation.base,   u_base,       vertx.base,   interp.base_t,   expr), 0.0);
    const auto height = max(valueFor(permutation.height, props.height, vertx.height, interp.height_t, expr), 0.0);

    const float3 normal = float3(vertx.normal_ed.xyz);
    const float edgedistance = vertx.normal_ed.w;
    const float t = glMod(normal.x, 2.0);
    const float z = (t != 0.0) ? height : base;     // TODO: This would come out wrong on GL for negative values, check it...
    const float4 position = fill.matrix * float4(float2(vertx.pos), z, 1);

    const auto pattern_from   = patternFor(permutation.pattern_from, tileProps.pattern_from,  vertx.pattern_from,   interp.pattern_from_t,     expr);
    const auto pattern_to     = patternFor(permutation.pattern_to,   tileProps.pattern_to,    vertx.pattern_to,     interp.pattern_to_t,       expr);

    if (permutation.overdrawInspector) {
        return {
            .position       = position,
            .lighting       = float4(1.0),
            .pattern_from   = float4(1.0),
            .pattern_to     = float4(1.0),
            .pos_a          = float2(1.0),
            .pos_b          = float2(1.0),
        };
    }

    float2 pattern_tl_a = pattern_from.xy;
    float2 pattern_br_a = pattern_from.zw;
    float2 pattern_tl_b = pattern_to.xy;
    float2 pattern_br_b = pattern_to.zw;

    float pixelRatio = fill.scale.x;
    float tileZoomRatio = fill.scale.y;
    float fromScale = fill.scale.z;
    float toScale = fill.scale.w;

    float2 display_size_a = float2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    float2 display_size_b = float2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    float2 pos = normal.x == 1.0 && normal.y == 0.0 && normal.z == 16384.0
        ? float2(vertx.pos) // extrusion top
        : float2(edgedistance, z * fill.height_factor); // extrusion side
    
    float4 lighting = float4(0.0, 0.0, 0.0, 1.0);
    float directional = clamp(dot(normal / 16383.0, props.light_position_base.xyz), 0.0, 1.0);
    directional = mix((1.0 - props.light_intensity), max((0.5 + props.light_intensity), 1.0), directional);

    if (normal.y != 0.0) {
        // This avoids another branching statement, but multiplies by a constant of 0.84 if no vertical gradient,
        // and otherwise calculates the gradient based on base + height
        directional *= (
            (1.0 - props.vertical_gradient) +
            (props.vertical_gradient * clamp((t + base) * pow(height / 150.0, 0.5), mix(0.7, 0.98, 1.0 - props.light_intensity), 1.0)));
    }

    lighting.rgb += clamp(directional * props.light_color_pad.rgb, mix(float3(0.0), float3(0.3), 1.0 - props.light_color_pad.rgb), float3(1.0));
    lighting *= props.opacity;
    
    return {
        .position       = position,
        .lighting       = lighting,
        .pattern_from   = pattern_from,
        .pattern_to     = pattern_to,
        .pos_a          = get_pattern_pos(fill.pixel_coord_upper, fill.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, pos),
        .pos_b          = get_pattern_pos(fill.pixel_coord_upper, fill.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, pos),
    };
}

fragment FragmentOutput fragmentMain(FragmentStage in [[stage_in]],
                                    device const FillExtrusionDrawableUBO& fill [[buffer(6)]],
                                    device const FillExtrusionDrawablePropsUBO& props [[buffer(7)]],
                                    device const FillExtrusionPermutationUBO& permutation [[buffer(10)]],
                                    texture2d<float, access::sample> image0 [[texture(0)]],
                                    sampler image0_sampler [[sampler(0)]]) {
    if (permutation.overdrawInspector) {
        return {half4(1.0)/*, in.position.z*/};
    }


    const float2 pattern_tl_a = in.pattern_from.xy;
    const float2 pattern_br_a = in.pattern_from.zw;
    const float2 pattern_tl_b = in.pattern_to.xy;
    const float2 pattern_br_b = in.pattern_to.zw;


    const float2 imagecoord = glMod(in.pos_a, 1.0);
    const float2 pos = mix(pattern_tl_a / fill.texsize, pattern_br_a / fill.texsize, imagecoord);
    const float4 color1 = image0.sample(image0_sampler, pos);

    const float2 imagecoord_b = glMod(in.pos_b, 1.0);
    const float2 pos2 = mix(pattern_tl_b / fill.texsize, pattern_br_b / fill.texsize, imagecoord_b);
    const float4 color2 = image0.sample(image0_sampler, pos2);

    return {half4(mix(color1, color2, props.fade) * in.lighting)/*, in.position.z*/};
}
)";
};

} // namespace shaders
} // namespace mbgl
