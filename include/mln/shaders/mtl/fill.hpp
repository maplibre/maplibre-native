#pragma once

#include <mln/shaders/fill_layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

constexpr auto fillShaderPrelude = R"(

enum {
    idFillDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idFillTilePropsUBO = drawableReservedUBOCount,
    idFillEvaluatedPropsUBO,
    fillUBOCount
};

//
// Fill

struct alignas(16) FillDrawableUBO {
    /*  0 */ float4x4 matrix;

    // Interpolations
    /* 64 */ float color_t;
    /* 68 */ float opacity_t;
    /* 72 */ float pad1;
    /* 76 */ float pad2;
    /* 80 */
};
static_assert(sizeof(FillDrawableUBO) == 5 * 16, "wrong size");

//
// Fill outline

struct alignas(16) FillOutlineDrawableUBO {
    /*  0 */ float4x4 matrix;

    // Interpolations
    /* 64 */ float outline_color_t;
    /* 68 */ float opacity_t;
    /* 72 */ float pad1;
    /* 76 */ float pad2;
    /* 80 */
};
static_assert(sizeof(FillOutlineDrawableUBO) == 5 * 16, "wrong size");

//
// Fill pattern

struct alignas(16) FillPatternDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float2 pixel_coord_upper;
    /* 72 */ float2 pixel_coord_lower;
    /* 80 */ float tile_ratio;

    // Interpolations
    /* 84 */ float pattern_from_t;
    /* 88 */ float pattern_to_t;
    /* 92 */ float opacity_t;
    /* 96 */
};
static_assert(sizeof(FillPatternDrawableUBO) == 6 * 16, "wrong size");

struct alignas(16) FillPatternTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */ float2 texsize;
    /* 40 */ float pad1;
    /* 44 */ float pad2;
    /* 48 */
};
static_assert(sizeof(FillPatternTilePropsUBO) == 3 * 16, "wrong size");

//
// Fill pattern outline

struct alignas(16) FillOutlinePatternDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float2 pixel_coord_upper;
    /* 72 */ float2 pixel_coord_lower;
    /* 80 */ float tile_ratio;

    // Interpolations
    /* 84 */ float pattern_from_t;
    /* 88 */ float pattern_to_t;
    /* 92 */ float opacity_t;
    /* 96 */
};
static_assert(sizeof(FillOutlinePatternDrawableUBO) == 6 * 16, "wrong size");

struct alignas(16) FillOutlinePatternTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */ float2 texsize;
    /* 40 */ float pad1;
    /* 44 */ float pad2;
    /* 48 */
};
static_assert(sizeof(FillOutlinePatternTilePropsUBO) == 3 * 16, "wrong size");

//
// Fill outline triangulated

struct alignas(16) FillOutlineTriangulatedDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float ratio;
    /* 68 */ float pad1;
    /* 72 */ float pad2;
    /* 76 */ float pad3;
    /* 80 */
};
static_assert(sizeof(FillOutlineTriangulatedDrawableUBO) == 5 * 16, "wrong size");

/// Evaluated properties that do not depend on the tile
struct alignas(16) FillEvaluatedPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float4 outline_color;
    /* 32 */ float opacity;
    /* 36 */ float fade;
    /* 40 */ float from_scale;
    /* 44 */ float to_scale;
    /* 48 */
};
static_assert(sizeof(FillEvaluatedPropsUBO) == 3 * 16, "wrong size");

union FillDrawableUnionUBO {
    FillDrawableUBO fillDrawableUBO;
    FillOutlineDrawableUBO fillOutlineDrawableUBO;
    FillPatternDrawableUBO fillPatternDrawableUBO;
    FillOutlinePatternDrawableUBO fillOutlinePatternDrawableUBO;
    FillOutlineTriangulatedDrawableUBO fillOutlineTriangulatedDrawableUBO;
};

union FillTilePropsUnionUBO {
    FillPatternTilePropsUBO fillPatternTilePropsUBO;
    FillOutlinePatternTilePropsUBO fillOutlinePatternTilePropsUBO;
};

float sdCircle(float2 p, float r) {
    return length(p) - r;
}

float sdPlane(float2 p, float2 n, float h)
{
    // n must be normalized
    return dot(p,n) + h;
}

float dot2(float2 v) {
    return dot(v,v);
}

float opUnion(float a, float b) {
    return min(a, b);
}

float opSmoothUnion(float d1, float d2, float k) {
    float h = clamp(0.5 + 0.5*(d2 - d1)/k, 0.0, 1.0);
    return mix(d2, d1, h) - k*h*(1.0 - h);
}

float opIntersection(float a, float b) {
    return max(a, b);
}

float opSubtraction(float a, float b) {
    return max(a, -b);
}

float2 bisector(float2 v0, float2 v1, float2 v2) {
    float2 v01 = normalize(v1 - v0);
    float2 v02 = normalize(v2 - v0);
    return normalize(v01 + v02);
}

float cotHalf(float cosTheta) {
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));
    return (1.0 + cosTheta) / sinTheta;
}

float constrainRadius(float2 v0, float2 v1, float2 v2, float radius) {
    // TODO don't recompute this
    float cosTheta0 = dot(normalize(v1 - v0), normalize(v2 - v0));
    float cosTheta1 = dot(normalize(v2 - v1), normalize(v0 - v1));
    float cosTheta2 = dot(normalize(v0 - v2), normalize(v1 - v2));

    float cot0 = cotHalf(cosTheta0);
    float cot1 = cotHalf(cosTheta1);
    float cot2 = cotHalf(cosTheta2);

    float r01 = length(v1 - v0) / (cot0 + cot1);
    float r02 = length(v2 - v0) / (cot0 + cot2);

    return min(min(radius, r01), r02);
}

void fitCircle(float2 v0, float2 v1, float2 v2, float distance, thread float &angle, thread float &radius) {
    float cosTheta = dot(normalize(v1 - v0), normalize(v2 - v0));
    float sinHalf = sqrt((1.0 - cosTheta) * 0.5);

    angle = sinHalf;
    radius = distance * sinHalf;
}

float dRoundedTri(float2 p, thread float &dTri, float2 v0, float2 v1, float2 v2, float distance) {
    // circle distance from vertex
    float angle;
    float radius;
    fitCircle(v0, v1, v2, distance, angle, radius);

    radius = constrainRadius(v0, v1, v2, radius);
    distance = radius / angle;

    // circle used to round it
    float2 bisect = bisector(v0, v1, v2);
    float2 circlePos = v0 + bisect * distance;
    float dCircle = sdCircle(p - circlePos, radius);

    // plane to isolate the two halfs of the circle (make the circle cutout asymmetrical)
    float dPlaneIn = sdPlane(p - circlePos, -bisect, -radius * angle);

    // mask based on plane
    dTri = opIntersection(dTri, dPlaneIn);

    return dCircle;
}

)";

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto source = R"(

struct PolygonVertex {
    short2 position;
    short2 prev_next;
};

struct TriangleIndex {
    uint value;
};

struct DataVertex {
#if !defined(HAS_UNIFORM_u_color)
    float color[4];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity[2];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 pos;

    float2 v0 [[flat]];
    float2 v1 [[flat]];
    float2 v2 [[flat]];

    float2 v0Prev [[flat]];
    float2 v0Next [[flat]];

    float2 v1Prev [[flat]];
    float2 v1Next [[flat]];

    float2 v2Prev [[flat]];
    float2 v2Next [[flat]];

    float external [[flat]];

#if !defined(HAS_UNIFORM_u_color)
    half4 color;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
};

FragmentStage vertex vertexMain(uint vertexID [[vertex_id]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillDrawableUnionUBO* drawableVector [[buffer(idFillDrawableUBO)]],
                                device const PolygonVertex* polygon [[buffer(fillUBOCount + 0)]],
                                device const TriangleIndex* indices [[buffer(fillUBOCount + 1)]],
                                device const DataVertex* data [[buffer(fillUBOCount + 2)]],
                                uint instanceID [[instance_id]]) {

    device const FillDrawableUBO& drawable = drawableVector[uboIndex].fillDrawableUBO;

    uint index = indices[instanceID * 3 + vertexID].value >> 2;
    float2 position = float2(polygon[index].position);

    uint i0 = indices[instanceID * 3 + 0].value >> 2;
    uint i1 = indices[instanceID * 3 + 1].value >> 2;
    uint i2 = indices[instanceID * 3 + 2].value >> 2;

    uint ignored0 = indices[instanceID * 3 + 0].value % 4 >> 1;
    uint ignored1 = indices[instanceID * 3 + 1].value % 4 >> 1;
    uint ignored2 = indices[instanceID * 3 + 2].value % 4 >> 1;

    uint external0 = indices[instanceID * 3 + 0].value % 2;
    uint external1 = indices[instanceID * 3 + 1].value % 2;
    uint external2 = indices[instanceID * 3 + 2].value % 2;

    uint i0Prev = 0;
    uint i0Next = 0;
    if (ignored0 == 0) {
        i0Prev = i0 + polygon[i0].prev_next[0];
        i0Next = i0 + polygon[i0].prev_next[1];
    }

    uint i1Prev = 0;
    uint i1Next = 0;
    if (ignored1 == 0) {
        i1Prev = i1 + polygon[i1].prev_next[0];
        i1Next = i1 + polygon[i1].prev_next[1];
    }

    uint i2Prev = 0;
    uint i2Next = 0;
    if (ignored2 == 0) {
        i2Prev = i2 + polygon[i2].prev_next[0];
        i2Next = i2 + polygon[i2].prev_next[1];
    }

    return {
        .position = drawable.matrix * float4(position, 0.0f, 1.0f),
        .pos = position,

        .v0 = float2(polygon[i0].position),
        .v1 = float2(polygon[i1].position),
        .v2 = float2(polygon[i2].position),

        .v0Prev = float2(polygon[i0Prev].position),
        .v0Next = float2(polygon[i0Next].position),

        .v1Prev = float2(polygon[i1Prev].position),
        .v1Next = float2(polygon[i1Next].position),

        .v2Prev = float2(polygon[i2Prev].position),
        .v2Next = float2(polygon[i2Next].position),

        .external = (external0 || external1 || external2) ? -1.0 : 1.0,

#if !defined(HAS_UNIFORM_u_color)
        .color    = half4(unpack_mix_color(data[index].color, drawable.color_t)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity  = half(unpack_mix_float(data[index].opacity, drawable.opacity_t)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]]) {
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

    float dTri = -1;

    if (in.v0Prev.x != in.v0Next.x || in.v0Prev.y != in.v0Next.y) {
        float d0 = dRoundedTri(in.pos, dTri, in.v0, in.v0Prev, in.v0Next, 20.0);
        dTri = opUnion(dTri, d0);
    }

    if (in.v1Prev.x != in.v1Next.x || in.v1Prev.y != in.v1Next.y) {
        float d1 = dRoundedTri(in.pos, dTri, in.v1, in.v1Prev, in.v1Next, 20.0);
        dTri = opUnion(dTri, d1);
    }

    if (in.v2Prev.x != in.v2Next.x || in.v2Prev.y != in.v2Next.y) {
        float d2 = dRoundedTri(in.pos, dTri, in.v2, in.v2Prev, in.v2Next, 20.0);
        dTri = opUnion(dTri, d2);
    }

    if (dTri * in.external > 0) {
        discard_fragment();
    }

    return half4(color * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "FillOutlineShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(0)]];
    float4 outline_color [[attribute(1)]];
    float2 opacity [[attribute(2)]];
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

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillDrawableUnionUBO* drawableVector [[buffer(idFillDrawableUBO)]]) {

    device const FillOutlineDrawableUBO& drawable = drawableVector[uboIndex].fillOutlineDrawableUBO;

    const float4 position = drawable.matrix * float4(float2(vertx.position), 0.0f, 1.0f);
    return {
        .position       = position,
        .pos            = (position.xy / position.w + 1.0) / 2.0 * paintParams.world_size,
#if !defined(HAS_UNIFORM_u_outline_color)
        .outline_color  = half4(unpack_mix_color(vertx.outline_color, drawable.outline_color_t)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity        = half(unpack_mix_float(vertx.opacity, drawable.opacity_t)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]]) {
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

    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(0)]];

#if !defined(HAS_UNIFORM_u_pattern_from)
    ushort4 pattern_from [[attribute(1)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    ushort4 pattern_to [[attribute(2)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(3)]];
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

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillDrawableUnionUBO* drawableVector [[buffer(idFillDrawableUBO)]],
                                device const FillTilePropsUnionUBO* tilePropsVector [[buffer(idFillTilePropsUBO)]],
                                device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]]) {

    device const FillPatternDrawableUBO& drawable = drawableVector[uboIndex].fillPatternDrawableUBO;
    device const FillPatternTilePropsUBO& tileProps = tilePropsVector[uboIndex].fillPatternTilePropsUBO;

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
        .opacity        = half(unpack_mix_float(vertx.opacity, drawable.opacity_t)),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const FillTilePropsUnionUBO* tilePropsVector [[buffer(idFillTilePropsUBO)]],
                            device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const FillPatternTilePropsUBO& tileProps = tilePropsVector[uboIndex].fillPatternTilePropsUBO;

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
    const float2 pos = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord);
    const float4 color1 = image0.sample(image0_sampler, pos);

    const float2 imagecoord_b = glMod(in.v_pos_b, 1.0);
    const float2 pos2 = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
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

    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(0)]];

#if !defined(HAS_UNIFORM_u_pattern_from)
    ushort4 pattern_from [[attribute(1)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    ushort4 pattern_to [[attribute(2)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(3)]];
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

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillDrawableUnionUBO* drawableVector [[buffer(idFillDrawableUBO)]],
                                device const FillTilePropsUnionUBO* tilePropsVector [[buffer(idFillTilePropsUBO)]],
                                device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]]) {

    device const FillOutlinePatternDrawableUBO& drawable = drawableVector[uboIndex].fillOutlinePatternDrawableUBO;
    device const FillOutlinePatternTilePropsUBO& tileProps = tilePropsVector[uboIndex].fillOutlinePatternTilePropsUBO;

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
    const auto opacity      = unpack_mix_float(vertx.opacity, drawable.opacity_t);
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
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const FillTilePropsUnionUBO* tilePropsVector [[buffer(idFillTilePropsUBO)]],
                            device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const FillOutlinePatternTilePropsUBO& tileProps = tilePropsVector[uboIndex].fillOutlinePatternTilePropsUBO;

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
    const float2 pos = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord);
    const float4 color1 = image0.sample(image0_sampler, pos);

    const float2 imagecoord_b = glMod(in.v_pos_b, 1.0);
    const float2 pos2 = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
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

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float width2;
    float2 normal;
    half gamma_scale;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const FillDrawableUnionUBO* drawableVector [[buffer(idFillDrawableUBO)]]) {

    device const FillOutlineTriangulatedDrawableUBO& drawable = drawableVector[uboIndex].fillOutlineTriangulatedDrawableUBO;

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
                            device const FillEvaluatedPropsUBO& props [[buffer(idFillEvaluatedPropsUBO)]]) {

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
} // namespace mln
