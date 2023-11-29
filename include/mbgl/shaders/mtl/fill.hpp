#pragma once

#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = true;

    static const std::array<AttributeInfo, 4> attributes;
    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(0)]];
    float4 color [[attribute(1)]];
    float2 opacity [[attribute(2)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
#if !defined(HAS_UNIFORM_u_color)
    half4 color;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
};

struct alignas(16) FillDrawableUBO {
    float4x4 matrix;
};

struct alignas(16) FillEvaluatedPropsUBO {
    float4 color;
    float opacity;
};

struct alignas(16) FillInterpolateUBO {
    float color_t;
    float opacity_t;
};

struct alignas(16) FillPermutationUBO {
    Attribute color;
    Attribute opacity;
    bool overdrawInspector;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillDrawableUBO& drawable [[buffer(3)]],
                                device const FillEvaluatedPropsUBO& props [[buffer(4)]],
                                device const FillInterpolateUBO& interp [[buffer(5)]],
                                device const FillPermutationUBO& permutation [[buffer(6)]],
                                device const ExpressionInputsUBO& expr [[buffer(7)]]) {
    return {
        .position = drawable.matrix * float4(float2(vertx.position), 0.0f, 1.0f),
#if !defined(HAS_UNIFORM_u_color)
        .color    = half4(colorFor(permutation.color, props.color, vertx.color, interp.color_t, expr)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity  = half(valueFor(permutation.opacity, props.opacity, vertx.opacity, interp.opacity_t, expr)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(4)]],
                            device const FillPermutationUBO& permutation [[buffer(6)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

#if defined(HAS_UNIFORM_u_color)
    const half4 color = half4(props.color);
#else
    const half4 color = in.color;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const half opacity = props.opacity;
#else
    const half opacity = in.opacity;
#endif

    return half4(color * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillOutlineShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = false;

    static const std::array<AttributeInfo, 4> attributes;
    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(0)]];
    float4 outline_color [[attribute(1)]];
    float2 opacity [[attribute(2)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 outline_color;
    float2 pos;
    half opacity;
};

struct alignas(16) FillOutlineDrawableUBO {
    float4x4 matrix;
    float2 world;
    float2 pad1;
};

struct alignas(16) FillOutlineEvaluatedPropsUBO {
    float4 outline_color;
    float opacity;
};

struct alignas(16) FillOutlineInterpolateUBO {
    float outline_color_t;
    float opacity_t;
};

struct alignas(16) FillOutlinePermutationUBO {
    Attribute outline_color;
    Attribute opacity;
    bool overdrawInspector;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillOutlineDrawableUBO& drawable [[buffer(3)]],
                                device const FillOutlineEvaluatedPropsUBO& props [[buffer(4)]],
                                device const FillOutlineInterpolateUBO& interp [[buffer(5)]],
                                device const FillOutlinePermutationUBO& permutation [[buffer(6)]],
                                device const ExpressionInputsUBO& expr [[buffer(7)]]) {

    const auto outline_color  = colorFor(permutation.outline_color,  props.outline_color,  vertx.outline_color,  interp.outline_color_t,  expr);
    const auto opacity        = valueFor(permutation.opacity,        props.opacity,        vertx.opacity,        interp.opacity_t,        expr);

    float4 position = drawable.matrix * float4(float2(vertx.position), 0.0f, 1.0f);
    float2 v_pos = (position.xy / position.w + 1.0) / 2.0 * drawable.world;

    return {
        .position       = position,
        .outline_color  = outline_color,
        .pos            = v_pos,
        .opacity        = half(opacity),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillOutlineEvaluatedPropsUBO& props [[buffer(4)]],
                            device const FillOutlinePermutationUBO& permutation [[buffer(6)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

//   TODO: Cause metal line primitive only support draw 1 pixel width line
//   use alpha to provide edge antialiased is no point
//   Should triangate the lines into triangles to support thick line and edge antialiased.
//    float dist = length(in.pos - in.position.xy);
//    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);

    return half4(in.outline_color * in.opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillPatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = false;

    static const std::array<AttributeInfo, 4> attributes;
    static const std::array<UniformBlockInfo, 6> uniforms;
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(0)]];
    ushort4 pattern_from [[attribute(1)]];
    ushort4 pattern_to [[attribute(2)]];
    float2 opacity [[attribute(3)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 pattern_from;
    float4 pattern_to;
    float2 v_pos_a;
    float2 v_pos_b;
    half opacity;
};

struct alignas(16) FillPatternDrawableUBO {
    float4x4 matrix;
    float4 scale;
    float2 pixel_coord_upper;
    float2 pixel_coord_lower;
    float2 texsize;
};

struct alignas(16) FillPatternEvaluatedPropsUBO {
    float opacity;
    float fade;
};

struct alignas(16) FillPatternTilePropsUBO {
    float4 pattern_from;
    float4 pattern_to;
};

struct alignas(16) FillPatternInterpolateUBO {
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
};

struct alignas(16) FillPatternPermutationUBO {
    Attribute pattern_from;
    Attribute pattern_to;
    Attribute opacity;
    bool overdrawInspector;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillPatternDrawableUBO& drawable [[buffer(4)]],
                                device const FillPatternTilePropsUBO& tileProps [[buffer(5)]],
                                device const FillPatternEvaluatedPropsUBO& props [[buffer(6)]],
                                device const FillPatternInterpolateUBO& interp [[buffer(7)]],
                                device const FillPatternPermutationUBO& permutation [[buffer(8)]],
                                device const ExpressionInputsUBO& expr [[buffer(9)]]) {

    const auto pattern_from   = patternFor(permutation.pattern_from, tileProps.pattern_from,  vertx.pattern_from,   interp.pattern_from_t,     expr);
    const auto pattern_to     = patternFor(permutation.pattern_to,   tileProps.pattern_to,    vertx.pattern_to,     interp.pattern_to_t,       expr);
    const auto opacity        = valueFor(permutation.opacity,        props.opacity,           vertx.opacity,        interp.opacity_t,        expr);

    float2 pattern_tl_a = pattern_from.xy;
    float2 pattern_br_a = pattern_from.zw;
    float2 pattern_tl_b = pattern_to.xy;
    float2 pattern_br_b = pattern_to.zw;

    float pixelRatio = drawable.scale.x;
    float tileZoomRatio = drawable.scale.y;
    float fromScale = drawable.scale.z;
    float toScale = drawable.scale.w;

    float2 display_size_a = float2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    float2 display_size_b = float2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);
    float2 postion = float2(vertx.position);

    return {
        .position       = drawable.matrix * float4(postion, 0, 1),
        .pattern_from   = pattern_from,
        .pattern_to     = pattern_to,
        .v_pos_a        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, postion),
        .v_pos_b        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, postion),
        .opacity        = half(opacity),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillPatternDrawableUBO& drawable [[buffer(4)]],
                            device const FillPatternEvaluatedPropsUBO& props [[buffer(6)]],
                            device const FillPatternPermutationUBO& permutation [[buffer(8)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    const float2 pattern_tl_a = in.pattern_from.xy;
    const float2 pattern_br_a = in.pattern_from.zw;
    const float2 pattern_tl_b = in.pattern_to.xy;
    const float2 pattern_br_b = in.pattern_to.zw;

    const float2 imagecoord = glMod(in.v_pos_a, 1.0);
    const float2 pos = mix(pattern_tl_a / drawable.texsize, pattern_br_a / drawable.texsize, imagecoord);
    const float4 color1 = image0.sample(image0_sampler, pos);

    const float2 imagecoord_b = glMod(in.v_pos_b, 1.0);
    const float2 pos2 = mix(pattern_tl_b / drawable.texsize, pattern_br_b / drawable.texsize, imagecoord_b);
    const float4 color2 = image0.sample(image0_sampler, pos2);

    return half4(mix(color1, color2, props.fade) * in.opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillOutlinePatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = false;

    static const std::array<AttributeInfo, 4> attributes;
    static const std::array<UniformBlockInfo, 6> uniforms;
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(0)]];
    ushort4 pattern_from [[attribute(1)]];
    ushort4 pattern_to [[attribute(2)]];
    float2 opacity [[attribute(3)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 v_pos_a;
    float2 v_pos_b;
    float4 pattern_from;
    float4 pattern_to;
    float2 v_pos;
    half opacity;
};

struct alignas(16) FillOutlinePatternDrawableUBO {
    float4x4 matrix;
    float4 scale;
    float2 world;
    float2 pixel_coord_upper;
    float2 pixel_coord_lower;
    float2 texsize;
};

struct alignas(16) FillOutlinePatternEvaluatedPropsUBO {
    float opacity;
    float fade;
};

struct alignas(16) FillOutlinePatternTilePropsUBO {
    float4 pattern_from;
    float4 pattern_to;
};

struct alignas(16) FillOutlinePatternInterpolateUBO {
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
};

struct alignas(16) FillOutlinePatternPermutationUBO {
    Attribute pattern_from;
    Attribute pattern_to;
    Attribute opacity;
    bool overdrawInspector;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillOutlinePatternDrawableUBO& drawable [[buffer(4)]],
                                device const FillOutlinePatternTilePropsUBO& tileProps [[buffer(5)]],
                                device const FillOutlinePatternEvaluatedPropsUBO& props [[buffer(6)]],
                                device const FillOutlinePatternInterpolateUBO& interp [[buffer(7)]],
                                device const FillOutlinePatternPermutationUBO& permutation [[buffer(8)]],
                                device const ExpressionInputsUBO& expr [[buffer(9)]]) {

    const auto pattern_from   = patternFor(permutation.pattern_from, tileProps.pattern_from,  vertx.pattern_from,   interp.pattern_from_t,     expr);
    const auto pattern_to     = patternFor(permutation.pattern_to,   tileProps.pattern_to,    vertx.pattern_to,     interp.pattern_to_t,       expr);
    const auto opacity        = valueFor(permutation.opacity,        props.opacity,           vertx.opacity,        interp.opacity_t,        expr);

    float2 pattern_tl_a = pattern_from.xy;
    float2 pattern_br_a = pattern_from.zw;
    float2 pattern_tl_b = pattern_to.xy;
    float2 pattern_br_b = pattern_to.zw;

    float pixelRatio = drawable.scale.x;
    float tileZoomRatio = drawable.scale.y;
    float fromScale = drawable.scale.z;
    float toScale = drawable.scale.w;

    float2 display_size_a = float2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    float2 display_size_b = float2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);
    float2 pos2 = float2(vertx.position);
    float4 position = drawable.matrix * float4(pos2, 0, 1);

    return {
        .position       = position,
        .pattern_from   = pattern_from,
        .pattern_to     = pattern_to,
        .v_pos_a        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, pos2),
        .v_pos_b        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, pos2),
        .v_pos          =  (position.xy / position.w + 1.0) / 2.0 * drawable.world,
        .opacity        = half(opacity),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillOutlinePatternDrawableUBO& drawable [[buffer(4)]],
                            device const FillOutlinePatternEvaluatedPropsUBO& props [[buffer(6)]],
                            device const FillOutlinePatternPermutationUBO& permutation [[buffer(8)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    const float2 pattern_tl_a = in.pattern_from.xy;
    const float2 pattern_br_a = in.pattern_from.zw;
    const float2 pattern_tl_b = in.pattern_to.xy;
    const float2 pattern_br_b = in.pattern_to.zw;

    const float2 imagecoord = glMod(in.v_pos_a, 1.0);
    const float2 pos = mix(pattern_tl_a / drawable.texsize, pattern_br_a / drawable.texsize, imagecoord);
    const float4 color1 = image0.sample(image0_sampler, pos);

    const float2 imagecoord_b = glMod(in.v_pos_b, 1.0);
    const float2 pos2 = mix(pattern_tl_b / drawable.texsize, pattern_br_b / drawable.texsize, imagecoord_b);
    const float4 color2 = image0.sample(image0_sampler, pos2);

    // TODO: Should triangate the lines into triangles to support thick line and edge antialiased.
    //float dist = length(in.v_pos - in.position.xy);
    //float alpha = 1.0 - smoothstep(0.0, 1.0, dist);

    return half4(mix(color1, color2, props.fade) * in.opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl
