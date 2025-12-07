#pragma once

#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto lineShadePrelude = R"(

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
    /* 96 */
};
static_assert(sizeof(LinePatternDrawableUBO) == 6 * 16, "wrong size");

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

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(lineUBOCount + 0)]];
    uchar4 data [[attribute(lineUBOCount + 1)]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color [[attribute(lineUBOCount + 2)]];
#endif
#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(lineUBOCount + 3)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(lineUBOCount + 4)]];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float2 gapwidth [[attribute(lineUBOCount + 5)]];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float2 offset [[attribute(lineUBOCount + 6)]];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float2 width [[attribute(lineUBOCount + 7)]];
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

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return PrecisionFloat4(1.0);
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

    return PrecisionFloat4(color * (alpha * opacity));
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

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(lineUBOCount + 0)]];
    uchar4 data [[attribute(lineUBOCount + 1)]];
#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(lineUBOCount + 2)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(lineUBOCount + 3)]];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float2 gapwidth [[attribute(lineUBOCount + 4)]];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float2 offset [[attribute(lineUBOCount + 5)]];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float2 width [[attribute(lineUBOCount + 6)]];
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

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            texture2d<float, access::sample> gradientTexture [[texture(0)]]) {
#if defined(OVERDRAW_INSPECTOR)
    return PrecisionFloat4(1.0);
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

    return PrecisionFloat4(color * (alpha * opacity));
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

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(lineUBOCount + 0)]];
    uchar4 data [[attribute(lineUBOCount + 1)]];

#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(lineUBOCount + 2)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(lineUBOCount + 3)]];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float2 gapwidth [[attribute(lineUBOCount + 4)]];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float2 offset [[attribute(lineUBOCount + 5)]];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float2 width [[attribute(lineUBOCount + 6)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_from)
    ushort4 pattern_from [[attribute(lineUBOCount + 7)]];
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    ushort4 pattern_to [[attribute(lineUBOCount + 8)]];
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
    PrecisionFloat4 pattern_from;
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
    PrecisionFloat4 pattern_to;
#endif
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
        .pattern_from = PrecisionFloat4(vertx.pattern_from),
#endif
#if !defined(HAS_UNIFORM_u_pattern_to)
        .pattern_to   = PrecisionFloat4(vertx.pattern_to),
#endif
    };
}

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const LineTilePropsUnionUBO* tilePropsVector [[buffer(idLineTilePropsUBO)]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {

#if defined(OVERDRAW_INSPECTOR)
    return PrecisionFloat4(1.0);
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

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (in.width2.y) or when fading out
    // (in.width2.x)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * in.gamma_scale;
    const float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0);

    const float x_a = glMod(in.linesofar / pattern_size_a.x, 1.0);
    const float x_b = glMod(in.linesofar / pattern_size_b.x, 1.0);

    // in.normal.y is 0 at the midpoint of the line, -1 at the lower edge, 1 at the upper edge
    // we clamp the line width outset to be between 0 and half the pattern height plus padding (2.0)
    // to ensure we don't sample outside the designated symbol on the sprite sheet.
    // 0.5 is added to shift the component to be bounded between 0 and 1 for interpolation of
    // the texture coordinate
    const float y_a = 0.5 + (in.normal.y * clamp(in.width2.x, 0.0, (pattern_size_a.y + 2.0) / 2.0) / pattern_size_a.y);
    const float y_b = 0.5 + (in.normal.y * clamp(in.width2.x, 0.0, (pattern_size_b.y + 2.0) / 2.0) / pattern_size_b.y);
    const float2 pos_a = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, float2(x_a, y_a));
    const float2 pos_b = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, float2(x_b, y_b));

    const float4 color = mix(image0.sample(image0_sampler, pos_a), image0.sample(image0_sampler, pos_b), tileProps.fade);

    return PrecisionFloat4(color * alpha * opacity);
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

    static constexpr auto prelude = lineShadePrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(lineUBOCount + 0)]];
    uchar4 data [[attribute(lineUBOCount + 1)]];

#if !defined(HAS_UNIFORM_u_color)
    float4 color [[attribute(lineUBOCount + 2)]];
#endif
#if !defined(HAS_UNIFORM_u_blur)
    float2 blur [[attribute(lineUBOCount + 3)]];
#endif
#if !defined(HAS_UNIFORM_u_opacity)
    float2 opacity [[attribute(lineUBOCount + 4)]];
#endif
#if !defined(HAS_UNIFORM_u_gapwidth)
    float2 gapwidth [[attribute(lineUBOCount + 5)]];
#endif
#if !defined(HAS_UNIFORM_u_offset)
    float2 offset [[attribute(lineUBOCount + 6)]];
#endif
#if !defined(HAS_UNIFORM_u_width)
    float2 width [[attribute(lineUBOCount + 7)]];
#endif
#if !defined(HAS_UNIFORM_u_floorwidth)
    float2 floorwidth [[attribute(lineUBOCount + 8)]];
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

PrecisionFloat4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const GlobalPaintParamsUBO& paintParams [[buffer(idGlobalPaintParamsUBO)]],
                            device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                            device const LineTilePropsUnionUBO* tilePropsVector [[buffer(idLineTilePropsUBO)]],
                            device const LineEvaluatedPropsUBO& props [[buffer(idLineEvaluatedPropsUBO)]],
                            device const LineExpressionUBO& expr [[buffer(idLineExpressionUBO)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {

#if defined(OVERDRAW_INSPECTOR)
    return PrecisionFloat4(1.0);
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

    return PrecisionFloat4(color * (alpha * opacity));
}
)";
};

} // namespace shaders
} // namespace mbgl
