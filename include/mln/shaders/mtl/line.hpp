#pragma once

#include <mln/shaders/line_layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

constexpr auto lineShaderPrelude = R"(

enum {
    idLineDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idLineTilePropsUBO = idDrawableReservedFragmentOnlyUBO,
    idLineEvaluatedPropsUBO = drawableReservedUBOCount,
    idLineExpressionUBO,
    lineUBOCount
};

//
// Line

struct alignas(16) LineDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float ratio;

    // Interpolations
    /* 68 */ float color_t;
    /* 72 */ float blur_t;
    /* 76 */ float opacity_t;
    /* 80 */ float gapwidth_t;
    /* 84 */ float offset_t;
    /* 88 */ float width_t;
    /* 92 */ float pad1;
    /* 96 */
};
static_assert(sizeof(LineDrawableUBO) == 6 * 16, "wrong size");

//
// Line gradient

struct alignas(16) LineGradientDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float ratio;

    // Interpolations
    /* 68 */ float blur_t;
    /* 72 */ float opacity_t;
    /* 76 */ float gapwidth_t;
    /* 80 */ float offset_t;
    /* 84 */ float width_t;
    /* 88 */ float pad1;
    /* 92 */ float pad2;
    /* 96 */
};
static_assert(sizeof(LineGradientDrawableUBO) == 6 * 16, "wrong size");

//
// Line pattern

struct alignas(16) LinePatternDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float ratio;

    // Interpolations
    /* 68 */ float blur_t;
    /* 72 */ float opacity_t;
    /* 76 */ float gapwidth_t;
    /* 80 */ float offset_t;
    /* 84 */ float width_t;
    /* 88 */ float pattern_from_t;
    /* 92 */ float pattern_to_t;
    /* 96 */ float floorwidth_t;
    /* 100 */ float pad1;
    /* 104 */ float pad2;
    /* 108 */ float pad3;
    /* 112 */
};
static_assert(sizeof(LinePatternDrawableUBO) == 7 * 16, "wrong size");

struct alignas(16) LinePatternTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */ float4 scale;
    /* 48 */ float2 texsize;
    /* 56 */ float fade;
    /* 60 */ float pad2;
    /* 64 */
};
static_assert(sizeof(LinePatternTilePropsUBO) == 4 * 16, "wrong size");

//
// Line SDF

struct alignas(16) LineSDFDrawableUBO {
    /*   0 */ float4x4 matrix;
    /*  64 */ float2 patternscale_a;
    /*  72 */ float2 patternscale_b;
    /*  80 */ float tex_y_a;
    /*  84 */ float tex_y_b;
    /*  88 */ float ratio;

    // Interpolations
    /*  92 */ float color_t;
    /*  96 */ float blur_t;
    /* 100 */ float opacity_t;
    /* 104 */ float gapwidth_t;
    /* 108 */ float offset_t;
    /* 112 */ float width_t;
    /* 116 */ float floorwidth_t;
    /* 120 */ float pad1;
    /* 124 */ float pad2;
    /* 128 */
};
static_assert(sizeof(LineSDFDrawableUBO) == 8 * 16, "wrong size");

struct alignas(16) LineSDFTilePropsUBO {
    /* 0 */ float sdfgamma;
    /* 4 */ float mix;
    /* 8 */ float pad1;
    /* 12 */ float pad2;
    /* 16 */
};
static_assert(sizeof(LineSDFTilePropsUBO) == 16, "wrong size");

/// Expression properties that do not depend on the tile
enum class LineExpressionMask : uint32_t {
    None = 0,
    Color = 1 << 0,
    Opacity = 1 << 1,
    Blur = 1 << 2,
    Width = 1 << 3,
    GapWidth = 1 << 4,
    FloorWidth = 1 << 5,
    Offset = 1 << 6,
};
bool operator&(LineExpressionMask a, LineExpressionMask b) { return (uint32_t)a & (uint32_t)b; }

struct alignas(16) LineExpressionUBO {
    GPUExpression color;
    GPUExpression blur;
    GPUExpression opacity;
    GPUExpression gapwidth;
    GPUExpression offset;
    GPUExpression width;
    GPUExpression floorwidth;
};
static_assert(sizeof(LineExpressionUBO) % 16 == 0, "wrong alignment");

/// Evaluated properties that do not depend on the tile
struct alignas(16) LineEvaluatedPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float blur;
    /* 20 */ float opacity;
    /* 24 */ float gapwidth;
    /* 28 */ float offset;
    /* 32 */ float width;
    /* 36 */ float floorwidth;
    /* 40 */ LineExpressionMask expressionMask;
    /* 44 */ float pad1;
    /* 48 */
};
static_assert(sizeof(LineEvaluatedPropsUBO) == 3 * 16, "wrong size");

union LineDrawableUnionUBO {
    LineDrawableUBO lineDrawableUBO;
    LineGradientDrawableUBO lineGradientDrawableUBO;
    LinePatternDrawableUBO linePatternDrawableUBO;
    LineSDFDrawableUBO lineSDFDrawableUBO;
};

union LineTilePropsUnionUBO {
    LinePatternTilePropsUBO linePatternTilePropsUBO;
    LineSDFTilePropsUBO lineSDFTilePropsUBO;
};

)";

template <>
struct ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LineShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = lineShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color [[attribute(2)]];
#endif
#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(3)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(4)]];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float2 gapwidth [[attribute(5)]];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float2 offset [[attribute(6)]];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float2 width [[attribute(7)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 width2;
    float2 normal;
    half gamma_scale;

#if !defined(HAS_UNIFORM_u_color)
    float4 color;
#endif
#if !defined(HAS_UNIFORM_u_blur)
    float blur;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const LineDrawableUnionUBO* drawableVector [[buffer(idLineDrawableUBO)]],
                                device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                                device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]]) {

    device const LineDrawableUBO& drawable = drawableVector[uboIndex].lineDrawableUBO;

#if defined(HAS_UNIFORM_u_gapwidth)
    const auto exprGapWidth = (props.expressionMask & LineExpressionMask::GapWidth);
    const auto gapwidth = (exprGapWidth ? expr.gapwidth.eval(paintParams.map_zoom) : props.gapwidth) / 2;
#else
    const auto gapwidth = unpack_mix_float(vertx.gapwidth, drawable.gapwidth_t) / 2;
#endif

#if defined(HAS_UNIFORM_u_offset)
    const auto exprOffset = (props.expressionMask & LineExpressionMask::Offset);
    const auto offset   = (exprOffset ? expr.offset.eval(paintParams.map_zoom) : props.offset) * -1;
#else
    const auto offset   = unpack_mix_float(vertx.offset, drawable.offset_t) * -1;
#endif

#if defined(HAS_UNIFORM_u_width)
    const auto exprWidth = (props.expressionMask & LineExpressionMask::Width);
    const auto width    = exprWidth ? expr.width.eval(paintParams.map_zoom) : props.width;
#else
    const auto width    = unpack_mix_float(vertx.width, drawable.width_t);
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float a_direction = glMod(float(vertx.data.z), 4.0) - 1.0;
    const float2 pos = floor(float2(vertx.pos_normal) * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of a_pos_normal
    const float2 normal = float2(vertx.pos_normal) - 2.0 * pos;
    const float2 v_normal = float2(normal.x, normal.y * 2.0 - 1.0);

    const float halfwidth = width / 2.0;
    const float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    const float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width of this vertex.
    const float2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    const float u = 0.5 * a_direction;
    const float t = 1.0 - abs(u);
    const float2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * float2x2(t, -u, u, t);

    const float4 projected_extrude = drawable.matrix * float4(dist / drawable.ratio, 0.0, 0.0);
    const float4 position = drawable.matrix * float4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    return {
        .position    = position,
        .width2      = float2(outset, inset),
        .normal      = v_normal,
        .gamma_scale = half(extrude_length_without_perspective / extrude_length_with_perspective),

#if !defined(HAS_UNIFORM_u_color)
        .color       = unpack_mix_color(vertx.color,   drawable.color_t),
#endif
#if !defined(HAS_UNIFORM_u_blur)
        .blur        = unpack_mix_float(vertx.blur,    drawable.blur_t),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity     = unpack_mix_float(vertx.opacity, drawable.opacity_t),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

#if defined(HAS_UNIFORM_u_color)
    const auto exprColor = (props.expressionMask & LineExpressionMask::Color);
    const auto color     = exprColor ? expr.color.evalColor(paintParams.map_zoom) : props.color;
#else
    const float4 color = in.color;
#endif

#if defined(HAS_UNIFORM_u_blur)
    const auto exprBlur = (props.expressionMask & LineExpressionMask::Blur);
    const float blur = exprBlur ? expr.blur.eval(paintParams.map_zoom) : props.blur;
#else
    const float blur = in.blur;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const auto exprOpacity = (props.expressionMask & LineExpressionMask::Opacity);
    const float opacity = exprOpacity ? expr.opacity.eval(paintParams.map_zoom) : props.opacity;
#else
    const float opacity = in.opacity;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (v_width2.y) or when fading out (v_width2.x)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * in.gamma_scale;
    const float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0);

    return half4(color * (alpha * opacity));
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineIntersectionShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LineIntersectionShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 7> attributes;
    static const std::array<AttributeInfo, 2> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = lineShaderPrelude;
    static constexpr auto source = R"(

float sdCircle(float2 p, float r) {
    return length(p) - r;
}

float sdPlane(float2 p, float2 n, float h) {
    // n must be normalized
    return dot(p, n) + h;
}

float dot2(float2 v) {
    return dot(v, v);
}

float opUnion(float a, float b) {
    return min(a, b);
}

float opSmoothUnion(float d1, float d2, float k) {
    float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return mix(d2, d1, h) - k * h * (1.0 - h);
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

void fitCircle(float2 v0, float2 v1, float2 v2, float radius, thread float &angle, thread float &distance) {
    float cosTheta = dot(normalize(v1 - v0), normalize(v2 - v0));
    float sinHalf = sqrt((1.0 - cosTheta) * 0.5);

    angle = sinHalf;
    distance = radius / sinHalf;
}

float dRoundedTri(float2 p, thread float &dTri, float2 v0, float2 v1, float2 v2, float radius) {
    // circle distance from vertex
    float angle;
    float distance;
    fitCircle(v0, v1, v2, radius, angle, distance);

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

float cross2(float2 a, float2 b) {
    return a.x * b.y - a.y * b.x;
}

float computeRadius(float2 prevPoint, float2 cornerPoint, float2 nextPoint, float desiredCornerDistance) {
    float maxEdgeLenPercent = 0.5;
    float sinParallelTreshold = sin(5 * M_PI_F / 180);

    float edge1Len = distance(prevPoint, cornerPoint);
    float edge2Len = distance(nextPoint, cornerPoint);
    if (edge1Len == 0.0 || edge2Len == 0.0) {
        return 0;
    }

    // Compute the start and end points of the rounded corner
    float2 edge1Vector = normalize(cornerPoint - prevPoint);
    float2 edge2Vector = normalize(nextPoint - cornerPoint);

    float edge1MaxCornerDistance = edge1Len * maxEdgeLenPercent;
    float edge2MaxCornerDistance = edge2Len * maxEdgeLenPercent;
    float cornerDistance = min3(desiredCornerDistance, edge1MaxCornerDistance, edge2MaxCornerDistance);

    float2 startPoint = cornerPoint - edge1Vector * cornerDistance;
    float2 endPoint = cornerPoint + edge2Vector * cornerDistance;

    // Perpendicular directions
    float2 perp1Vector = float2(-edge1Vector.y, edge1Vector.x);
    float2 perp2Vector = float2(-edge2Vector.y, edge2Vector.x);

    // Ensure perpendiculars point toward same side
    if (cross2(edge1Vector, edge2Vector) < 0) {
        perp1Vector *= -1.0;
        perp2Vector *= -1.0;
    }

    // Center = intersection of perpendiculars
    float perpCrossProduct = cross2(perp1Vector, perp2Vector);
    if (abs(perpCrossProduct) < sinParallelTreshold) {
        return 0;
    }
    float t = cross2(endPoint - startPoint, perp2Vector) / perpCrossProduct;
    float2 centerPoint = startPoint + perp1Vector * t;

    float radius = distance(startPoint, centerPoint);
    return radius;
}

struct VertexStage {
    uint3 triangle_instance [[attribute(7)]];
    uint2 data_indices [[attribute(8)]];

    /*short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];*/
};

// Same order as in LinePaintProperties
struct DataVertex {
#if !defined(HAS_UNIFORM_u_blur)
    float blur[2];
#endif
#if !defined(HAS_UNIFORM_u_color)
    float color[4];
#endif
#if !defined(HAS_UNIFORM_u_floorwidth)
    float floorwidth[2];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float gapwidth[2];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float offset[2];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity[2];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    ushort pattern_to[4];
#endif
#if !defined(HAS_UNIFORM_u_pattern_from)
    ushort pattern_from[4];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float width[2];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];

    float2 triangle_pos;

    float2 triangle_corner_pos [[flat]];
    float2 triangle_corner_prev [[flat]];
    float2 triangle_corner_next [[flat]];
    float triangle_corner_radius [[flat]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color1 [[flat]];
    float4 color2 [[flat]];
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    float opacity1 [[flat]];
    float opacity2 [[flat]];
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const LineDrawableUnionUBO* drawableVector [[buffer(idLineDrawableUBO)]],
                                device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                                device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]],
                                device const DataVertex* data [[buffer(lineUBOCount + 1)]],
                                uint vertexID [[vertex_id]]) {

    device const LineDrawableUBO& drawable = drawableVector[uboIndex].lineDrawableUBO;

    float2 triangle_vertex_pos[3];
    float triangle_corner_radius;

    for (int i = 0; i < 3; i++) {
        int16_t pos_x = int16_t(vertx.triangle_instance[i] >> 16);
        int16_t pos_y = int16_t(vertx.triangle_instance[i] & 0xFFFF);
        triangle_vertex_pos[i] = float2(pos_x, pos_y);
    }
    
    float2 edgePrev = normalize(triangle_vertex_pos[0] - triangle_vertex_pos[1]);
    float2 edgeNext = normalize(triangle_vertex_pos[2] - triangle_vertex_pos[1]);
    float sinAngle = cross2(edgeNext, edgePrev);

    for (int i = 0; i < 2; i++) {
        uint dataIndex = vertx.data_indices[i] / 2;
        float direction = (float(vertx.data_indices[i] % 2) * 2.0 - 1.0);

#if defined(HAS_UNIFORM_u_gapwidth)
        const auto exprGapWidth = (props.expressionMask & LineExpressionMask::GapWidth);
        const auto gapwidth = (exprGapWidth ? expr.gapwidth.eval(paintParams.map_zoom) : props.gapwidth) / 2;
#else
        const auto gapwidth = unpack_mix_float(data[dataIndex].gapwidth, drawable.gapwidth_t) / 2;
#endif

#if defined(HAS_UNIFORM_u_offset)
        const auto exprOffset = (props.expressionMask & LineExpressionMask::Offset);
        const auto offset   = (exprOffset ? expr.offset.eval(paintParams.map_zoom) : props.offset);
#else
        const auto offset   = unpack_mix_float(data[dataIndex].offset, drawable.offset_t);
#endif

#if defined(HAS_UNIFORM_u_width)
        const auto exprWidth = (props.expressionMask & LineExpressionMask::Width);
        const auto width    = exprWidth ? expr.width.eval(paintParams.map_zoom) : props.width;
#else
        const auto width    = unpack_mix_float(data[dataIndex].width, drawable.width_t);
#endif

#if defined(HAS_UNIFORM_u_blur)
        const auto exprBlur = (props.expressionMask & LineExpressionMask::Width);
        const auto blur     = exprBlur ? expr.blur.eval(paintParams.map_zoom) : props.blur;
#else
        const auto blur     = unpack_mix_float(data[dataIndex].blur, drawable.blur_t);
#endif

        const float halfwidth = width / 2.0;
        const float outset = max(gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) - blur, 0.0);

        float2 outsetVector = (i == 0 ? edgeNext : edgePrev);
        float2 offsetVector = (i == 0 ? -edgeNext : edgePrev);
        const float2 dist = (outset * outsetVector + offset * direction * offsetVector) / sinAngle;

        triangle_vertex_pos[i] = triangle_vertex_pos[i] + dist / drawable.ratio;
        triangle_vertex_pos[i + 1] = triangle_vertex_pos[i + 1] + dist / drawable.ratio;
    }

    triangle_corner_radius = computeRadius(triangle_vertex_pos[0], triangle_vertex_pos[1], triangle_vertex_pos[2], 20);

    float2 position = triangle_vertex_pos[vertexID];
    uint dataIndex1 = vertx.data_indices[0] / 2;
    uint dataIndex2 = vertx.data_indices[1] / 2;

    return {
        .position = drawable.matrix * float4(position, 0.0f, 1.0f),
        
        .triangle_pos = position,

        .triangle_corner_prev = triangle_vertex_pos[0],
        .triangle_corner_pos = triangle_vertex_pos[1],
        .triangle_corner_next = triangle_vertex_pos[2],
        .triangle_corner_radius = triangle_corner_radius,

#if !defined(HAS_UNIFORM_u_color) 
        .color1        = unpack_mix_color(data[dataIndex1].color, drawable.color_t),
        .color2        = unpack_mix_color(data[dataIndex2].color, drawable.color_t),
#endif
#if !defined(HAS_UNIFORM_u_opacity) 
        .opacity1      = unpack_mix_float(data[dataIndex1].opacity, drawable.opacity_t),
        .opacity2      = unpack_mix_float(data[dataIndex2].opacity, drawable.opacity_t),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

#if !defined(HAS_UNIFORM_u_color) || !defined(HAS_UNIFORM_u_opacity)
    float2 vPos = in.triangle_pos - in.triangle_corner_pos;
    float2 vPrev = in.triangle_corner_prev - in.triangle_corner_pos;
    float2 vNext = in.triangle_corner_next - in.triangle_corner_pos;
    
    float angle = atan2(cross2(vPos, vPrev), dot(vPos, vPrev));
    float angleMax = atan2(cross2(vNext, vPrev), dot(vNext, vPrev));
#endif

#if defined(HAS_UNIFORM_u_color)
    const auto exprColor = (props.expressionMask & LineExpressionMask::Color);
    const auto color     = exprColor ? expr.color.evalColor(paintParams.map_zoom) : props.color;
#else
    const float4 color = mix(in.color1, in.color2, angle / angleMax);
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const auto exprOpacity = (props.expressionMask & LineExpressionMask::Opacity);
    const float opacity = exprOpacity ? expr.opacity.eval(paintParams.map_zoom) : props.opacity;
#else
    const float opacity = mix(in.opacity1, in.opacity2, angle / angleMax);
#endif

    float dTri = -1.0;

    if (in.triangle_corner_radius > 0.0) {
        float d0 = dRoundedTri(in.triangle_pos, dTri, in.triangle_corner_pos, in.triangle_corner_prev, in.triangle_corner_next, in.triangle_corner_radius);
        dTri = opUnion(dTri, d0);
    }

    if (dTri < 0.0) {
        return half4(0.0);
    }

    return half4(color * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LineGradientShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 7> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = lineShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];
#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(2)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(3)]];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float2 gapwidth [[attribute(4)]];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float2 offset [[attribute(5)]];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float2 width [[attribute(6)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 width2;
    half2 normal;
    half gamma_scale;
    float lineprogress;

#if !defined(HAS_UNIFORM_u_blur)
    half blur;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const LineDrawableUnionUBO* drawableVector [[buffer(idLineDrawableUBO)]],
                                device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]]) {

    device const LineGradientDrawableUBO& drawable = drawableVector[uboIndex].lineGradientDrawableUBO;

#if !defined(HAS_UNIFORM_u_blur)
    const auto blur     = unpack_mix_float(vertx.blur,     drawable.blur_t);
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    const auto opacity  = unpack_mix_float(vertx.opacity,  drawable.opacity_t);
#endif
#if defined(HAS_UNIFORM_u_gapwidth)
    const auto gapwidth = props.gapwidth / 2;
#else
    const auto gapwidth = unpack_mix_float(vertx.gapwidth, drawable.gapwidth_t) / 2;
#endif
#if defined(HAS_UNIFORM_u_offset)
    const auto offset   = props.offset * -1;
#else
    const auto offset   = unpack_mix_float(vertx.offset,   drawable.offset_t) * -1;
#endif
#if defined(HAS_UNIFORM_u_width)
    const auto width    = props.width;
#else
    const auto width    = unpack_mix_float(vertx.width,    drawable.width_t);
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float a_direction = glMod(float(vertx.data.z), 4.0) - 1.0;
    const float v_lineprogress = (floor(float(vertx.data.z) / 4.0) + vertx.data.w * 64.0) * 2.0 / MAX_LINE_DISTANCE;
    const float2 pos = floor(float2(vertx.pos_normal) * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of a_pos_normal
    const float2 normal = float2(vertx.pos_normal) - 2.0 * pos;
    const float2 v_normal = float2(normal.x, normal.y * 2.0 - 1.0);

    const float halfwidth = width / 2.0;
    const float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    const float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width of this vertex.
    const float2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    const float u = 0.5 * a_direction;
    const float t = 1.0 - abs(u);
    const float2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * float2x2(t, -u, u, t);

    const float4 projected_extrude = drawable.matrix * float4(dist / drawable.ratio, 0.0, 0.0);
    const float4 position = drawable.matrix * float4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    return {
        .position     = position,
        .width2       = float2(outset, inset),
        .normal       = half2(v_normal),
        .gamma_scale  = half(extrude_length_without_perspective / extrude_length_with_perspective),
        .lineprogress = v_lineprogress,

#if !defined(HAS_UNIFORM_u_blur)
        .blur         = half(blur),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity      = half(opacity),
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> gradientTexture [[texture(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

#if defined(HAS_UNIFORM_u_blur)
    const float blur = props.blur;
#else
    const float blur = float(in.blur);
#endif
#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = float(in.opacity);
#endif

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (v_width2.y) or when fading out (v_width2.x)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * in.gamma_scale;
    const float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0);

    // For gradient lines, v_lineprogress is the ratio along the entire line,
    // scaled to [0, 2^15), and the gradient ramp is stored in a texture.
    constexpr sampler sampler2d(coord::normalized, filter::linear);
    const float4 color = gradientTexture.sample(sampler2d, float2(in.lineprogress, 0.5));

    return half4(color * (alpha * opacity));
}
)";
};

template <>
struct ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LinePatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = lineShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];

#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(2)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(3)]];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float2 gapwidth [[attribute(4)]];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float2 offset [[attribute(5)]];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float2 width [[attribute(6)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_from)
    ushort4 pattern_from [[attribute(7)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    ushort4 pattern_to [[attribute(8)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 width2;
    float linesofar;
    half2 normal;
    half gamma_scale;

#if !defined(HAS_UNIFORM_u_blur)
    half blur;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    half opacity;
#endif
#if !defined(HAS_UNIFORM_u_pattern_from)
    half4 pattern_from;
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    half4 pattern_to;
#endif
    float floorwidth;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const LineDrawableUnionUBO* drawableVector [[buffer(idLineDrawableUBO)]],
                                device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                                device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]]) {

    device const LinePatternDrawableUBO& drawable = drawableVector[uboIndex].linePatternDrawableUBO;

#if defined(HAS_UNIFORM_u_gapwidth)
    const auto exprGapWidth = (props.expressionMask & LineExpressionMask::GapWidth);
    const auto gapwidth = (exprGapWidth ? expr.gapwidth.eval(paintParams.map_zoom) : props.gapwidth) / 2;
#else
    const auto gapwidth = unpack_mix_float(vertx.gapwidth, drawable.gapwidth_t) / 2;
#endif

#if defined(HAS_UNIFORM_u_offset)
    const auto exprOffset = (props.expressionMask & LineExpressionMask::Offset);
    const auto offset   = (exprOffset ? expr.offset.eval(paintParams.map_zoom) : props.offset) * -1;
#else
    const auto offset   = unpack_mix_float(vertx.offset, drawable.offset_t) * -1;
#endif

#if defined(HAS_UNIFORM_u_width)
    const auto exprWidth = (props.expressionMask & LineExpressionMask::Width);
    const auto width    = exprWidth ? expr.width.eval(paintParams.map_zoom) : props.width;
#else
    const auto width    = unpack_mix_float(vertx.width, drawable.width_t);
#endif

#if defined(HAS_UNIFORM_u_width)
    const auto exprFloorWidth = (props.expressionMask & LineExpressionMask::FloorWidth);
    const auto floorwidth = exprFloorWidth ? expr.floorwidth.eval(paintParams.map_zoom) : props.floorwidth;
#else
    const auto floorwidth = unpack_mix_float(vertx.width, drawable.floorwidth_t);
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float a_direction = glMod(float(vertx.data.z), 4.0) - 1.0;
    const float linesofar = (floor(vertx.data.z / 4.0) + vertx.data.w * 64.0) * LINE_DISTANCE_SCALE;
    const float2 pos = floor(float2(vertx.pos_normal) * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of a_pos_normal
    const float2 normal = float2(vertx.pos_normal) - 2.0 * pos;
    const float2 v_normal = float2(normal.x, normal.y * 2.0 - 1.0);

    const float halfwidth = width / 2.0;
    const float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    const float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width of this vertex.
    const float2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    const float u = 0.5 * a_direction;
    const float t = 1.0 - abs(u);
    const float2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * float2x2(t, -u, u, t);

    const float4 projected_extrude = drawable.matrix * float4(dist / drawable.ratio, 0.0, 0.0);
    const float4 position = drawable.matrix * float4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    return {
        .position     = position,
        .width2       = float2(outset, inset),
        .normal       = half2(v_normal),
        .gamma_scale  = half(extrude_length_without_perspective / extrude_length_with_perspective),
        .linesofar    = linesofar,

#if !defined(HAS_UNIFORM_u_blur)
        .blur         = half(unpack_mix_float(vertx.blur, drawable.blur_t)),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity      = half(unpack_mix_float(vertx.opacity, drawable.opacity_t)),
#endif
#if !defined(HAS_UNIFORM_u_pattern_from)
        .pattern_from = half4(vertx.pattern_from),
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
        .pattern_to   = half4(vertx.pattern_to),
#endif
        .floorwidth   = floorwidth,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const LineTilePropsUnionUBO* tilePropsVector [[buffer(idLineTilePropsUBO)]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {

#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const LinePatternTilePropsUBO& tileProps = tilePropsVector[uboIndex].linePatternTilePropsUBO;

#if defined(HAS_UNIFORM_u_blur)
    const auto exprBlur = (props.expressionMask & LineExpressionMask::Blur);
    const float blur = exprBlur ? expr.blur.eval(paintParams.map_zoom) : props.blur;
#else
    const float blur = in.blur;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const auto exprOpacity = (props.expressionMask & LineExpressionMask::Opacity);
    const float opacity = exprOpacity ? expr.opacity.eval(paintParams.map_zoom) : props.opacity;
#else
    const float opacity = in.opacity;
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const auto pattern_from = float4(tileProps.pattern_from);
#else
    const auto pattern_from = float4(in.pattern_from);
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const auto pattern_to   = float4(tileProps.pattern_to);
#else
    const auto pattern_to   = float4(in.pattern_to);
#endif

    const float2 pattern_tl_a = pattern_from.xy;
    const float2 pattern_br_a = pattern_from.zw;
    const float2 pattern_tl_b = pattern_to.xy;
    const float2 pattern_br_b = pattern_to.zw;

    const float pixelRatio = tileProps.scale.x;
    const float tileZoomRatio = tileProps.scale.y;
    const float fromScale = tileProps.scale.z;
    const float toScale = tileProps.scale.w;

    const float2 display_size_a = float2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const float2 display_size_b = float2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    const float2 pattern_size_a = float2(display_size_a.x * fromScale / tileZoomRatio, display_size_a.y);
    const float2 pattern_size_b = float2(display_size_b.x * toScale / tileZoomRatio, display_size_b.y);

    const float aspect_a = display_size_a.y / in.floorwidth;
    const float aspect_b = display_size_b.y / in.floorwidth;

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (in.width2.y) or when fading out
    // (in.width2.x)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * in.gamma_scale;
    const float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0);

    const float x_a = glMod(in.linesofar / pattern_size_a.x * aspect_a, 1.0);
    const float x_b = glMod(in.linesofar / pattern_size_b.x * aspect_b, 1.0);

    const float y = 0.5 * in.normal.y + 0.5;

    const float2 texel_size = 1.0 / tileProps.texsize;
    const float2 half_texel = 0.5 * texel_size;
    float2 pos_a = mix(pattern_tl_a * texel_size - texel_size, pattern_br_a * texel_size + texel_size, float2(x_a, y));
    pos_a = clamp(pos_a, (pattern_tl_a - 1.0) * texel_size + half_texel, (pattern_br_a + 1.0) * texel_size - half_texel);
    float2 pos_b = mix(pattern_tl_b * texel_size - texel_size, pattern_br_b * texel_size + texel_size, float2(x_b, y));
    pos_b = clamp(pos_b, (pattern_tl_b - 1.0) * texel_size + half_texel, (pattern_br_b + 1.0) * texel_size - half_texel);

    const float4 color = mix(image0.sample(image0_sampler, pos_a), image0.sample(image0_sampler, pos_b), tileProps.fade);

    return half4(color * alpha * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LineSDFShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = lineShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color [[attribute(2)]];
#endif
#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(3)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(4)]];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float2 gapwidth [[attribute(5)]];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float2 offset [[attribute(6)]];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float2 width [[attribute(7)]];
#endif
#if !defined(HAS_UNIFORM_u_floorwidth)
    float2 floorwidth [[attribute(8)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 width2;
    float2 normal;
    float2 tex_a;
    float2 tex_b;
    half gamma_scale;

#if !defined(HAS_UNIFORM_u_color)
    float4 color;
#endif
#if !defined(HAS_UNIFORM_u_blur)
    float blur;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity;
#endif
#if !defined(HAS_UNIFORM_u_floorwidth)
    float floorwidth;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const LineDrawableUnionUBO* drawableVector [[buffer(idLineDrawableUBO)]],
                                device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                                device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]]) {

    device const LineSDFDrawableUBO& drawable = drawableVector[uboIndex].lineSDFDrawableUBO;

#if defined(HAS_UNIFORM_u_gapwidth)
    const auto exprGapWidth = (props.expressionMask & LineExpressionMask::GapWidth);
    const auto gapwidth = (exprGapWidth ? expr.gapwidth.eval(paintParams.map_zoom) : props.gapwidth) / 2.0;
#else
    const auto gapwidth = unpack_mix_float(vertx.gapwidth, drawable.gapwidth_t) / 2.0;
#endif

#if defined(HAS_UNIFORM_u_offset)
    const auto exprOffset = (props.expressionMask & LineExpressionMask::Offset);
    const auto offset   = (exprOffset ? expr.offset.eval(paintParams.map_zoom) : props.offset) * -1.0;
#else
    const auto offset   = unpack_mix_float(vertx.offset, drawable.offset_t) * -1.0;
#endif

#if defined(HAS_UNIFORM_u_width)
    const auto exprWidth = (props.expressionMask & LineExpressionMask::Width);
    const auto width    = exprWidth ? expr.width.eval(paintParams.map_zoom) : props.width;
#else
    const auto width    = unpack_mix_float(vertx.width, drawable.width_t);
#endif

#if defined(HAS_UNIFORM_u_floorwidth)
    const auto exprFloorWidth = (props.expressionMask & LineExpressionMask::FloorWidth);
    const auto floorwidth = exprFloorWidth ? expr.floorwidth.eval(paintParams.map_zoom) : props.floorwidth;
#else
    const auto floorwidth = unpack_mix_float(vertx.floorwidth, drawable.floorwidth_t);
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float a_direction = glMod(float(vertx.data.z), 4.0) - 1.0;
    float linesofar = (floor(float(vertx.data.z) / 4.0) + float(vertx.data.w) * 64.0) * LINE_DISTANCE_SCALE;
    const float2 pos = floor(float2(vertx.pos_normal) * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of a_pos_normal
    const float2 normal = float2(vertx.pos_normal) - 2.0 * pos;
    const float2 v_normal = float2(normal.x, normal.y * 2.0 - 1.0);

    const float halfwidth = width / 2.0;
    const float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    const float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width of this vertex.
    const float2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    const float u = 0.5 * a_direction;
    const float t = 1.0 - abs(u);
    const float2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * float2x2(t, -u, u, t);

    const float4 projected_extrude = drawable.matrix * float4(dist / drawable.ratio, 0.0, 0.0);
    const float4 position = drawable.matrix * float4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    return {
        .position     = position,
        .width2       = float2(outset, inset),
        .normal       = v_normal,
        .gamma_scale  = half(extrude_length_without_perspective / extrude_length_with_perspective),
        .tex_a        = float2(linesofar * drawable.patternscale_a.x / floorwidth, v_normal.y * drawable.patternscale_a.y + drawable.tex_y_a),
        .tex_b        = float2(linesofar * drawable.patternscale_b.x / floorwidth, v_normal.y * drawable.patternscale_b.y + drawable.tex_y_b),

#if !defined(HAS_UNIFORM_u_color)
        .color        = unpack_mix_color(vertx.color, drawable.color_t),
#endif
#if !defined(HAS_UNIFORM_u_blur)
        .blur         = unpack_mix_float(vertx.blur, drawable.blur_t),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity      = unpack_mix_float(vertx.opacity, drawable.opacity_t),
#endif
#if !defined(HAS_UNIFORM_u_floorwidth)
        .floorwidth   = floorwidth,
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const LineTilePropsUnionUBO* tilePropsVector [[buffer(idLineTilePropsUBO)]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {

#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const LineSDFTilePropsUBO& tileProps = tilePropsVector[uboIndex].lineSDFTilePropsUBO;

#if defined(HAS_UNIFORM_u_color)
    const auto exprColor = (props.expressionMask & LineExpressionMask::Color);
    const auto color     = exprColor ? expr.color.evalColor(paintParams.map_zoom) : props.color;
#else
    const float4 color = in.color;
#endif

#if defined(HAS_UNIFORM_u_blur)
    const auto exprBlur = (props.expressionMask & LineExpressionMask::Blur);
    const float blur = exprBlur ? expr.blur.eval(paintParams.map_zoom) : props.blur;
#else
    const float blur = in.blur;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const auto exprOpacity = (props.expressionMask & LineExpressionMask::Opacity);
    const float opacity = exprOpacity ? expr.opacity.eval(paintParams.map_zoom) : props.opacity;
#else
    const float opacity = in.opacity;
#endif

#if defined(HAS_UNIFORM_u_floorwidth)
    const auto exprFloorWidth = (props.expressionMask & LineExpressionMask::FloorWidth);
    const auto floorwidth = exprFloorWidth ? expr.floorwidth.eval(paintParams.map_zoom) : props.floorwidth;
#else
    const auto floorwidth = in.floorwidth;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (`v_width2.y`) or when fading out (`v_width2.x`)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * in.gamma_scale;

    const float sdfdist_a = image0.sample(image0_sampler, in.tex_a).a;
    const float sdfdist_b = image0.sample(image0_sampler, in.tex_b).a;
    const float sdfdist = mix(sdfdist_a, sdfdist_b, tileProps.mix);
    const float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0) *
                        smoothstep(0.5 - tileProps.sdfgamma / floorwidth, 0.5 + tileProps.sdfgamma / floorwidth, sdfdist);

    return half4(color * (alpha * opacity));
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineGradientSDFShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LineGradientSDFShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto prelude = lineShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];

#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(2)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(3)]];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float2 gapwidth [[attribute(4)]];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float2 offset [[attribute(5)]];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float2 width [[attribute(6)]];
#endif
#if !defined(HAS_UNIFORM_u_floorwidth)
    float2 floorwidth [[attribute(7)]];
#endif
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 width2;
    float2 normal;
    float2 tex_a;
    float2 tex_b;
    half gamma_scale;
    float lineprogress;

#if !defined(HAS_UNIFORM_u_blur)
    float blur;
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float opacity;
#endif
#if !defined(HAS_UNIFORM_u_floorwidth)
    float floorwidth;
#endif
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const LineDrawableUnionUBO* drawableVector [[buffer(idLineDrawableUBO)]],
                                device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                                device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]]) {

    device const LineSDFDrawableUBO& drawable = drawableVector[uboIndex].lineSDFDrawableUBO;

#if defined(HAS_UNIFORM_u_gapwidth)
    const auto exprGapWidth = (props.expressionMask & LineExpressionMask::GapWidth);
    const auto gapwidth = (exprGapWidth ? expr.gapwidth.eval(paintParams.map_zoom) : props.gapwidth) / 2.0;
#else
    const auto gapwidth = unpack_mix_float(vertx.gapwidth, drawable.gapwidth_t) / 2.0;
#endif

#if defined(HAS_UNIFORM_u_offset)
    const auto exprOffset = (props.expressionMask & LineExpressionMask::Offset);
    const auto offset   = (exprOffset ? expr.offset.eval(paintParams.map_zoom) : props.offset) * -1.0;
#else
    const auto offset   = unpack_mix_float(vertx.offset, drawable.offset_t) * -1.0;
#endif

#if defined(HAS_UNIFORM_u_width)
    const auto exprWidth = (props.expressionMask & LineExpressionMask::Width);
    const auto width    = exprWidth ? expr.width.eval(paintParams.map_zoom) : props.width;
#else
    const auto width    = unpack_mix_float(vertx.width, drawable.width_t);
#endif

#if defined(HAS_UNIFORM_u_floorwidth)
    const auto exprFloorWidth = (props.expressionMask & LineExpressionMask::FloorWidth);
    const auto floorwidth = exprFloorWidth ? expr.floorwidth.eval(paintParams.map_zoom) : props.floorwidth;
#else
    const auto floorwidth = unpack_mix_float(vertx.floorwidth, drawable.floorwidth_t);
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float a_direction = glMod(float(vertx.data.z), 4.0) - 1.0;
    const float linesofar = (floor(float(vertx.data.z) / 4.0) + float(vertx.data.w) * 64.0) * LINE_DISTANCE_SCALE;
    const float v_lineprogress = (floor(float(vertx.data.z) / 4.0) + float(vertx.data.w) * 64.0) * 2.0 / MAX_LINE_DISTANCE;
    const float2 pos = floor(float2(vertx.pos_normal) * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of a_pos_normal
    const float2 normal = float2(vertx.pos_normal) - 2.0 * pos;
    const float2 v_normal = float2(normal.x, normal.y * 2.0 - 1.0);

    const float halfwidth = width / 2.0;
    const float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
    const float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width of this vertex.
    const float2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    const float u = 0.5 * a_direction;
    const float t = 1.0 - abs(u);
    const float2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * float2x2(t, -u, u, t);

    const float4 projected_extrude = drawable.matrix * float4(dist / drawable.ratio, 0.0, 0.0);
    const float4 position = drawable.matrix * float4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * paintParams.units_to_pixels);

    return {
        .position     = position,
        .width2       = float2(outset, inset),
        .normal       = v_normal,
        .tex_a        = float2(linesofar * drawable.patternscale_a.x / floorwidth, v_normal.y * drawable.patternscale_a.y + drawable.tex_y_a),
        .tex_b        = float2(linesofar * drawable.patternscale_b.x / floorwidth, v_normal.y * drawable.patternscale_b.y + drawable.tex_y_b),
        .gamma_scale  = half(extrude_length_without_perspective / extrude_length_with_perspective),
        .lineprogress = v_lineprogress,

#if !defined(HAS_UNIFORM_u_blur)
        .blur         = unpack_mix_float(vertx.blur, drawable.blur_t),
#endif
#if !defined(HAS_UNIFORM_u_opacity)
        .opacity      = unpack_mix_float(vertx.opacity, drawable.opacity_t),
#endif
#if !defined(HAS_UNIFORM_u_floorwidth)
        .floorwidth   = floorwidth,
#endif
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const LineTilePropsUnionUBO* tilePropsVector [[buffer(idLineTilePropsUBO)]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]],
                            texture2d<float, access::sample> gradientTexture [[texture(0)]],
                            texture2d<float, access::sample> dashTexture [[texture(1)]],
                            sampler gradientSampler [[sampler(0)]],
                            sampler dashSampler [[sampler(1)]]) {

#if defined(OVERDRAW_INSPECTOR)
    return half4(1.0);
#endif

    device const LineSDFTilePropsUBO& tileProps = tilePropsVector[uboIndex].lineSDFTilePropsUBO;

#if defined(HAS_UNIFORM_u_blur)
    const auto exprBlur = (props.expressionMask & LineExpressionMask::Blur);
    const float blur = exprBlur ? expr.blur.eval(paintParams.map_zoom) : props.blur;
#else
    const float blur = in.blur;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const auto exprOpacity = (props.expressionMask & LineExpressionMask::Opacity);
    const float opacity = exprOpacity ? expr.opacity.eval(paintParams.map_zoom) : props.opacity;
#else
    const float opacity = in.opacity;
#endif

#if defined(HAS_UNIFORM_u_floorwidth)
    const auto exprFloorWidth = (props.expressionMask & LineExpressionMask::FloorWidth);
    const auto floorwidth = exprFloorWidth ? expr.floorwidth.eval(paintParams.map_zoom) : props.floorwidth;
#else
    const auto floorwidth = in.floorwidth;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (`v_width2.y`) or when fading out (`v_width2.x`)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * in.gamma_scale;

    const float sdfdist_a = dashTexture.sample(dashSampler, in.tex_a).a;
    const float sdfdist_b = dashTexture.sample(dashSampler, in.tex_b).a;
    const float sdfdist = mix(sdfdist_a, sdfdist_b, tileProps.mix);
    const float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0) *
                        smoothstep(0.5 - tileProps.sdfgamma / floorwidth, 0.5 + tileProps.sdfgamma / floorwidth, sdfdist);

    // The gradient ramp is stored in a texture indexed by the ratio along the entire line.
    const float4 color = gradientTexture.sample(gradientSampler, float2(in.lineprogress, 0.5));

    return half4(color * (alpha * opacity));
}
)";
};

} // namespace shaders
} // namespace mln
