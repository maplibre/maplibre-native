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

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(5)]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color [[attribute(6)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(7)]];
#endif
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

struct alignas(16) FillInterpolateUBO {
    float color_t;
    float opacity_t;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const FillDrawableUBO& drawable [[buffer(1)]],
                                device const FillInterpolateUBO& interp [[buffer(3)]],
                                device const FillEvaluatedPropsUBO& props [[buffer(4)]]) {
    return {
        .position = drawable.matrix * float4(float2(vertx.position), 0.0f, 1.0f),
#if !defined(HAS_UNIFORM_u_color)
        .color    = half4(unpack_mix_color(vertx.color, interp.color_t)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity  = half(unpack_mix_float(vertx.opacity, interp.opacity_t)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(4)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

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

    static const std::array<UniformBlockInfo, 4> uniforms;
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(
struct VertexStage {
    short2 position [[attribute(5)]];
    float4 outline_color [[attribute(6)]];
    float2 opacity [[attribute(7)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;
#if !defined(HAS_UNIFORM_u_outline_color)
    half4 outline_color;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
};

struct alignas(16) FillOutlineDrawableUBO {
    float4x4 matrix;
};

struct alignas(16) FillOutlineInterpolateUBO {
    float outline_color_t;
    float opacity_t;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(0)]],
                                device const FillOutlineDrawableUBO& drawable [[buffer(1)]],
                                device const FillOutlineInterpolateUBO& interp [[buffer(3)]],
                                device const FillEvaluatedPropsUBO& props [[buffer(4)]]) {
    const float4 position = drawable.matrix * float4(float2(vertx.position), 0.0f, 1.0f);
    return {
        .position       = position,
        .pos            = (position.xy / position.w + 1.0) / 2.0 * paintParams.world_size,
#if !defined(HAS_UNIFORM_u_outline_color)
        .outline_color  = half4(unpack_mix_color(vertx.outline_color, interp.outline_color_t)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity        = half(unpack_mix_float(vertx.opacity, interp.opacity_t)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(4)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

//   TODO: Cause metal line primitive only support draw 1 pixel width line
//   use alpha to provide edge antialiased is no point
//   Should triangate the lines into triangles to support thick line and edge antialiased.
//    float dist = length(in.pos - in.position.xy);
//    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);

#if defined(HAS_UNIFORM_u_outline_color)
    const half4 color = half4(props.outline_color);
#else
    const half4 color = in.outline_color;
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
struct ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillPatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(
struct VertexStage {
    short2 position [[attribute(5)]];

#if !defined(HAS_UNIFORM_u_pattern_from)
    ushort4 pattern_from [[attribute(6)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    ushort4 pattern_to [[attribute(7)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(8)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 v_pos_a;
    float2 v_pos_b;

#if !defined(HAS_UNIFORM_u_pattern_from)
    half4 pattern_from;
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    half4 pattern_to;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
};

struct alignas(16) FillPatternDrawableUBO {
    float4x4 matrix;
    float2 pixel_coord_upper;
    float2 pixel_coord_lower;
    float2 texsize;
    float tile_ratio;
    float pad;
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

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(0)]],
                                device const FillPatternDrawableUBO& drawable [[buffer(1)]],
                                device const FillPatternTilePropsUBO& tileProps [[buffer(2)]],
                                device const FillPatternInterpolateUBO& interp [[buffer(3)]],
                                device const FillEvaluatedPropsUBO& props [[buffer(4)]]) {
#if defined(HAS_UNIFORM_u_pattern_from)
    const auto pattern_from = float4(tileProps.pattern_from);
#else
    const auto pattern_from = float4(vertx.pattern_from);
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const auto pattern_to   = float4(tileProps.pattern_to);
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
    const float2 postion = float2(vertx.position);

    return {
        .position       = drawable.matrix * float4(postion, 0, 1),
        .v_pos_a        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, postion),
        .v_pos_b        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, postion),
#if !defined(HAS_UNIFORM_u_pattern_from)
        .pattern_from   = half4(pattern_from),
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
        .pattern_to     = half4(pattern_to),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity        = half(unpack_mix_float(vertx.opacity, interp.opacity_t)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillPatternDrawableUBO& drawable [[buffer(1)]],
                            device const FillPatternTilePropsUBO& tileProps [[buffer(2)]],
                            device const FillEvaluatedPropsUBO& props [[buffer(4)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const auto pattern_from   = float4(tileProps.pattern_from);
#else
    const auto pattern_from   = float4(in.pattern_from);
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const auto pattern_to     = float4(tileProps.pattern_to);
#else
    const auto pattern_to     = float4(in.pattern_to);
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const auto opacity        = props.opacity;
#else
    const auto opacity        = in.opacity;
#endif

    const float2 pattern_tl_a = pattern_from.xy;
    const float2 pattern_br_a = pattern_from.zw;
    const float2 pattern_tl_b = pattern_to.xy;
    const float2 pattern_br_b = pattern_to.zw;

    const float2 imagecoord = glMod(in.v_pos_a, 1.0);
    const float2 pos = mix(pattern_tl_a / drawable.texsize, pattern_br_a / drawable.texsize, imagecoord);
    const float4 color1 = image0.sample(image0_sampler, pos);

    const float2 imagecoord_b = glMod(in.v_pos_b, 1.0);
    const float2 pos2 = mix(pattern_tl_b / drawable.texsize, pattern_br_b / drawable.texsize, imagecoord_b);
    const float4 color2 = image0.sample(image0_sampler, pos2);

    return half4(mix(color1, color2, props.fade) * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillOutlinePatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(5)]];

#if !defined(HAS_UNIFORM_u_pattern_from)
    ushort4 pattern_from [[attribute(6)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    ushort4 pattern_to [[attribute(7)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(8)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 v_pos_a;
    float2 v_pos_b;
    float2 v_pos;

#if !defined(HAS_UNIFORM_u_pattern_from)
    half4 pattern_from;
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    half4 pattern_to;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
};

struct alignas(16) FillOutlinePatternDrawableUBO {
    float4x4 matrix;
    float2 pixel_coord_upper;
    float2 pixel_coord_lower;
    float2 texsize;
    float tile_ratio;
    float pad;
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

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(0)]],
                                device const FillOutlinePatternDrawableUBO& drawable [[buffer(1)]],
                                device const FillOutlinePatternTilePropsUBO& tileProps [[buffer(2)]],
                                device const FillOutlinePatternInterpolateUBO& interp [[buffer(3)]],
                                device const FillEvaluatedPropsUBO& props [[buffer(4)]]) {
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

#if !defined(HAS_UNIFORM_u_opacity)
    const auto opacity      = unpack_mix_float(vertx.opacity, interp.opacity_t);
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
    const float2 pos2 = float2(vertx.position);
    const float4 position = drawable.matrix * float4(pos2, 0, 1);

    return {
        .position       = position,
        .v_pos_a        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, pos2),
        .v_pos_b        = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, pos2),
        .v_pos          = (position.xy / position.w + 1.0) / 2.0 * paintParams.world_size,

#if !defined(HAS_UNIFORM_u_pattern_from)
        .pattern_from   = half4(pattern_from),
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
        .pattern_to     = half4(pattern_to),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity        = half(opacity),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillOutlinePatternDrawableUBO& drawable [[buffer(1)]],
                            device const FillOutlinePatternTilePropsUBO& tileProps [[buffer(2)]],
                            device const FillEvaluatedPropsUBO& props [[buffer(4)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const auto pattern_from   = float4(tileProps.pattern_from);
#else
    const auto pattern_from   = float4(in.pattern_from);
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const auto pattern_to     = float4(tileProps.pattern_to);
#else
    const auto pattern_to     = float4(in.pattern_to);
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const auto opacity        = props.opacity;
#else
    const auto opacity        = in.opacity;
#endif

    const float2 pattern_tl_a = pattern_from.xy;
    const float2 pattern_br_a = pattern_from.zw;
    const float2 pattern_tl_b = pattern_to.xy;
    const float2 pattern_br_b = pattern_to.zw;

    const float2 imagecoord = glMod(in.v_pos_a, 1.0);
    const float2 pos = mix(pattern_tl_a / drawable.texsize, pattern_br_a / drawable.texsize, imagecoord);
    const float4 color1 = image0.sample(image0_sampler, pos);

    const float2 imagecoord_b = glMod(in.v_pos_b, 1.0);
    const float2 pos2 = mix(pattern_tl_b / drawable.texsize, pattern_br_b / drawable.texsize, imagecoord_b);
    const float4 color2 = image0.sample(image0_sampler, pos2);

    // TODO: Should triangate the lines into triangles to support thick line and edge antialiased.
    //float dist = length(in.v_pos - in.position.xy);
    //float alpha = 1.0 - smoothstep(0.0, 1.0, dist);

    return half4(mix(color1, color2, props.fade) * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillOutlineTriangulatedShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(
struct VertexStage {
    short2 pos_normal [[attribute(5)]];
    uchar4 data [[attribute(6)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float width2;
    float2 normal;
    half gamma_scale;
};

struct alignas(16) FillOutlineTriangulatedDrawableUBO {
    float4x4 matrix;
    float ratio;
    float pad1, pad2, pad3;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(0)]],
                                device const FillOutlineTriangulatedDrawableUBO& drawable [[buffer(1)]],
                                device const FillEvaluatedPropsUBO& props [[buffer(4)]]) {

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float2 pos = floor(float2(vertx.pos_normal) * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of a_pos_normal
    const float2 normal = float2(vertx.pos_normal) - 2.0 * pos;
    const float2 v_normal = float2(normal.x, normal.y * 2.0 - 1.0);

    const float width = 1.0; //Should come from props ubo
    const float halfwidth = width / 2.0;
    const float outset = halfwidth + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width of this vertex.
    const float2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    const float4 projected_extrude = drawable.matrix * float4(dist / drawable.ratio, 0.0, 0.0);
    const float4 position = drawable.matrix * float4(pos, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    return {
        .position    = position,
        .width2      = outset,
        .normal      = v_normal,
        .gamma_scale = half(extrude_length_without_perspective / extrude_length_with_perspective),
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(4)]]) {

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (`v_width2.y`) or when fading out (`v_width2.x`)
    const float blur2 = (1.0 / DEVICE_PIXEL_RATIO) * in.gamma_scale;
    const float alpha = clamp(min(dist + blur2, in.width2 - dist) / blur2, 0.0, 1.0);

    return half4(props.outline_color * (alpha * props.opacity));
}
)";
};

} // namespace shaders
} // namespace mbgl
