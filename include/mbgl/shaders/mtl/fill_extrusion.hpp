#pragma once

#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto fillExtrusionShaderPrelude = R"(

enum {
    idFillExtrusionDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idFillExtrusionTilePropsUBO = drawableReservedUBOCount,
    idFillExtrusionPropsUBO,
    fillExtrusionUBOCount
};

struct alignas(16) FillExtrusionDrawableUBO {
    /*   0 */ float4x4 matrix;
    /*  64 */ float2 pixel_coord_upper;
    /*  72 */ float2 pixel_coord_lower;
    /*  80 */ float height_factor;
    /*  84 */ float tile_ratio;

    // Interpolations
    /*  88 */ float base_t;
    /*  92 */ float height_t;
    /*  96 */ float color_t;
    /* 100 */ float pattern_from_t;
    /* 104 */ float pattern_to_t;
    /* 108 */ float pad1;
    /* 112 */
};
static_assert(sizeof(FillExtrusionDrawableUBO) == 7 * 16, "wrong size");

struct alignas(16) FillExtrusionTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */ float2 texsize;
    /* 40 */ float pad1;
    /* 44 */ float pad2;
    /* 48 */
};
static_assert(sizeof(FillExtrusionTilePropsUBO) == 3 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) FillExtrusionPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float4 light_color_pad;
    /* 32 */ float4 light_position_base;
    /* 48 */ float height;
    /* 52 */ float light_intensity;
    /* 56 */ float vertical_gradient;
    /* 60 */ float opacity;
    /* 64 */ float fade;
    /* 68 */ float from_scale;
    /* 72 */ float to_scale;
    /* 76 */ float pad2;
    /* 80 */
};
static_assert(sizeof(FillExtrusionPropsUBO) == 5 * 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillExtrusionShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(fillExtrusionUBOCount + 0)]];
    short4 normal_ed [[attribute(fillExtrusionUBOCount + 1)]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color [[attribute(fillExtrusionUBOCount + 2)]];
#endif
#if !defined(HAS_UNIFORM_u_base)
    float base [[attribute(fillExtrusionUBOCount + 3)]];
#endif
#if !defined(HAS_UNIFORM_u_height)
    float height [[attribute(fillExtrusionUBOCount + 4)]];
#endif
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
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillExtrusionDrawableUBO* drawableVector [[buffer(idFillExtrusionDrawableUBO)]],
                                device const FillExtrusionPropsUBO& props [[buffer(idFillExtrusionPropsUBO)]]) {

    device const FillExtrusionDrawableUBO& drawable = drawableVector[uboIndex];

#if defined(HAS_UNIFORM_u_base)
    const auto base   = props.light_position_base.w;
#else
    const auto base   = max(unpack_mix_float(vertx.base, drawable.base_t), 0.0);
#endif
#if defined(HAS_UNIFORM_u_height)
    const auto height = props.height;
#else
    const auto height = max(unpack_mix_float(vertx.height, drawable.height_t), 0.0);
#endif

    const float3 normal = float3(vertx.normal_ed.xyz);
    const float t = glMod(normal.x, 2.0);
    const float z = (t != 0.0) ? height : base;     // TODO: This would come out wrong on GL for negative values, check it...
    const float4 position = drawable.matrix * float4(float2(vertx.pos), z, 1);

#if defined(OVERDRAW_INSPECTOR)
    return {
        .position = position,
        .color    = half4(1.0),
    };
#endif

#if defined(HAS_UNIFORM_u_color)
    auto color = props.color;
#else
    auto color = unpack_mix_color(vertx.color, drawable.color_t);
#endif

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

template <>
struct ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillExtrusionPatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 6> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = fillExtrusionShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos [[attribute(fillExtrusionUBOCount + 0)]];
    short4 normal_ed [[attribute(fillExtrusionUBOCount + 1)]];

#if !defined(HAS_UNIFORM_u_base)
    float base [[attribute(fillExtrusionUBOCount + 2)]];
#endif
#if !defined(HAS_UNIFORM_u_height)
    float height [[attribute(fillExtrusionUBOCount + 3)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_from)
    ushort4 pattern_from [[attribute(fillExtrusionUBOCount + 4)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    ushort4 pattern_to [[attribute(fillExtrusionUBOCount + 5)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 lighting;
    float2 pos_a;
    float2 pos_b;

#if !defined(HAS_UNIFORM_u_pattern_from)
    half4 pattern_from;
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    half4 pattern_to;
#endif
};

struct FragmentOutput {
    half4 color [[color(0)]];
    //float depth [[depth(less)]]; // Write depth value if it's less than what's already there
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillExtrusionDrawableUBO* drawableVector [[buffer(idFillExtrusionDrawableUBO)]],
                                device const FillExtrusionTilePropsUBO* tilePropsVector [[buffer(idFillExtrusionTilePropsUBO)]],
                                device const FillExtrusionPropsUBO& props [[buffer(idFillExtrusionPropsUBO)]]) {

    device const FillExtrusionDrawableUBO& drawable = drawableVector[uboIndex];
    device const FillExtrusionTilePropsUBO& tileProps = tilePropsVector[uboIndex];

#if defined(HAS_UNIFORM_u_base)
    const auto base   = props.light_position_base.w;
#else
    const auto base   = max(unpack_mix_float(vertx.base, drawable.base_t), 0.0);
#endif
#if defined(HAS_UNIFORM_u_height)
    const auto height = props.height;
#else
    const auto height = max(unpack_mix_float(vertx.height, drawable.height_t), 0.0);
#endif

    const float3 normal = float3(vertx.normal_ed.xyz);
    const float edgedistance = vertx.normal_ed.w;
    const float t = glMod(normal.x, 2.0);
    const float z = (t != 0.0) ? height : base;     // TODO: This would come out wrong on GL for negative values, check it...
    const float4 position = drawable.matrix * float4(float2(vertx.pos), z, 1);

#if defined(OVERDRAW_INSPECTOR)
    return {
        .position       = position,
        .lighting       = float4(1.0),
        .pattern_from   = float4(1.0),
        .pattern_to     = float4(1.0),
        .pos_a          = float2(1.0),
        .pos_b          = float2(1.0),
    };
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const auto pattern_from = tileProps.pattern_from;
#else
    const auto pattern_from = float4(vertx.pattern_from);
#endif
#if defined(HAS_UNIFORM_u_pattern_to)
    const auto pattern_to   = tileProps.pattern_to;
#else
    const auto pattern_to   = float4(vertx.pattern_to);
#endif

    const float2 pattern_tl_a = pattern_from.xy;
    const float2 pattern_br_a = pattern_from.zw;
    const float2 pattern_tl_b = pattern_to.xy;
    const float2 pattern_br_b = pattern_to.zw;

    const float pixelRatio = paintParams.pixel_ratio;
    const float tileZoomRatio = drawable.tile_ratio;
    const float fromScale = props.from_scale;
    const float toScale = props.to_scale;

    const float2 display_size_a = float2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const float2 display_size_b = float2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    const float2 pos = normal.x == 1.0 && normal.y == 0.0 && normal.z == 16384.0
        ? float2(vertx.pos) // extrusion top
        : float2(edgedistance, z * drawable.height_factor); // extrusion side

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
#if !defined(HAS_UNIFORM_u_pattern_from)
        .pattern_from   = half4(pattern_from),
#endif
#if !defined(HAS_UNIFORM_u_pattern_from)
        .pattern_to     = half4(pattern_to),
#endif
        .pos_a          = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, pos),
        .pos_b          = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, pos),
    };
}

fragment FragmentOutput fragmentMain(FragmentStage in [[stage_in]],
                                     device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                     device const FillExtrusionTilePropsUBO* tilePropsVector [[buffer(idFillExtrusionTilePropsUBO)]],
                                     device const FillExtrusionPropsUBO& props [[buffer(idFillExtrusionPropsUBO)]],
                                     texture2d<float, access::sample> image0 [[texture(0)]],
                                     sampler image0_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return {half4(1.0)/*, in.position.z*/};
#endif

    device const FillExtrusionTilePropsUBO& tileProps = tilePropsVector[uboIndex];

#if defined(HAS_UNIFORM_u_pattern_from)
    const auto pattern_from = float4(tileProps.pattern_from);
#else
    const auto pattern_from = float4(in.pattern_from);
#endif
#if defined(HAS_UNIFORM_u_pattern_to)
    const auto pattern_to = float4(tileProps.pattern_to);
#else
    const auto pattern_to = float4(in.pattern_to);
#endif

    const float2 pattern_tl_a = pattern_from.xy;
    const float2 pattern_br_a = pattern_from.zw;
    const float2 pattern_tl_b = pattern_to.xy;
    const float2 pattern_br_b = pattern_to.zw;

    const float2 imagecoord = glMod(in.pos_a, 1.0);
    const float2 pos = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord);
    const float4 color1 = image0.sample(image0_sampler, pos);

    const float2 imagecoord_b = glMod(in.pos_b, 1.0);
    const float2 pos2 = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    const float4 color2 = image0.sample(image0_sampler, pos2);

    return {half4(mix(color1, color2, props.fade) * in.lighting)/*, in.position.z*/};
}
)";
};

} // namespace shaders
} // namespace mbgl
