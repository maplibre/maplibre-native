#pragma once

#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LineShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 8> attributes;
    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];
    float4 color [[attribute(2)]];
    float2 blur [[attribute(3)]];
    float2 opacity [[attribute(4)]];
    float2 gapwidth [[attribute(5)]];
    float2 offset [[attribute(6)]];
    float2 width [[attribute(7)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 width2;
    float2 normal;
    float gamma_scale;
    float4 color;
    float blur;
    float opacity;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const LineUBO& line [[buffer(8)]],
                                device const LinePropertiesUBO& props [[buffer(9)]],
                                device const LineInterpolationUBO& interp [[buffer(10)]],
                                device const LinePermutationUBO& permutation [[buffer(11)]],
                                device const ExpressionInputsUBO& expr [[buffer(12)]]) {

    const auto color    = colorFor(permutation.color,    props.color,    vertx.color,    interp.color_t,    expr);
    const auto blur     = valueFor(permutation.blur,     props.blur,     vertx.blur,     interp.blur_t,     expr);
    const auto opacity  = valueFor(permutation.opacity,  props.opacity,  vertx.opacity,  interp.opacity_t,  expr);
    const auto gapwidth = valueFor(permutation.gapwidth, props.gapwidth, vertx.gapwidth, interp.gapwidth_t, expr) / 2;
    const auto offset   = valueFor(permutation.offset,   props.offset,   vertx.offset,   interp.offset_t,   expr) * -1;
    const auto width    = valueFor(permutation.width,    props.width,    vertx.width,    interp.width_t,    expr);

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / line.device_pixel_ratio / 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float a_direction = fmod(float(vertx.data.z), 4.0) - 1.0;
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

    const float4 projected_extrude = line.matrix * float4(dist / line.ratio, 0.0, 0.0);
    const float4 position = line.matrix * float4(pos + offset2 / line.ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * line.units_to_pixels);

    return {
        .position    = position,
        .width2      = float2(outset, inset),
        .normal      = v_normal,
        .gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective,
        .color       = color,
        .blur        = blur,
        .opacity     = opacity,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const LineUBO& line [[buffer(8)]],
                            device const LinePermutationUBO& permutation [[buffer(11)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (v_width2.y) or when fading out (v_width2.x)
    const float blur2 = (in.blur + 1.0 / line.device_pixel_ratio) * in.gamma_scale;
    const float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0);

    return half4(in.color * (alpha * in.opacity));
}
)";
};

template <>
struct ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LinePatternShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 9> attributes;
    static const std::array<UniformBlockInfo, 6> uniforms;
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];
    float2 blur [[attribute(2)]];
    float2 opacity [[attribute(3)]];
    float2 gapwidth [[attribute(4)]];
    float2 offset [[attribute(5)]];
    float2 width [[attribute(6)]];
    ushort4 pattern_from [[attribute(7)]];
    ushort4 pattern_to [[attribute(8)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 width2;
    float2 normal;
    float gamma_scale;
    float blur;
    float opacity;
    float4 pattern_from;
    float4 pattern_to;
    float linesofar;
};

struct alignas(16) LinePatternUBO {
    float4x4 matrix;
    float4 scale;
    float2 texsize;
    float2 units_to_pixels;
    float ratio;
    float device_pixel_ratio;
    float fade;
    float pad1;
};

struct alignas(16) LinePatternPropertiesUBO {
    float blur;
    float opacity;
    float offset;
    float gapwidth;
    float width;
    float pad1, pad2, pad3;
};

struct alignas(16) LinePatternInterpolationUBO {
    float blur_t;
    float opacity_t;
    float offset_t;
    float gapwidth_t;
    float width_t;
    float pattern_from_t;
    float pattern_to_t;
    float pad1;
};

struct alignas(16) LinePatternTilePropertiesUBO {
    float4 pattern_from;
    float4 pattern_to;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const LinePatternUBO& line [[buffer(9)]],
                                device const LinePatternPropertiesUBO& props [[buffer(10)]],
                                device const LinePatternInterpolationUBO& interp [[buffer(11)]],
                                device const LinePatternTilePropertiesUBO& tileProps [[buffer(12)]],
                                device const LinePermutationUBO& permutation [[buffer(13)]],
                                device const ExpressionInputsUBO& expr [[buffer(14)]]) {

    const auto blur     = valueFor(permutation.blur,     props.blur,     vertx.blur,     interp.blur_t,     expr);
    const auto opacity  = valueFor(permutation.opacity,  props.opacity,  vertx.opacity,  interp.opacity_t,  expr);
    const auto gapwidth = valueFor(permutation.gapwidth, props.gapwidth, vertx.gapwidth, interp.gapwidth_t, expr) / 2;
    const auto offset   = valueFor(permutation.offset,   props.offset,   vertx.offset,   interp.offset_t,   expr) * -1;
    const auto width    = valueFor(permutation.width,    props.width,    vertx.width,    interp.width_t,    expr);

    const auto pattern_from   = patternFor(permutation.pattern_from,    tileProps.pattern_from,  vertx.pattern_from,   interp.pattern_from_t,     expr);
    const auto pattern_to     = patternFor(permutation.pattern_to,      tileProps.pattern_to,    vertx.pattern_to,     interp.pattern_to_t,       expr);

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / line.device_pixel_ratio / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float a_direction = fmod(float(vertx.data.z), 4.0) - 1.0;
    float linesofar = (floor(vertx.data.z / 4.0) + vertx.data.w * 64.0) * LINE_DISTANCE_SCALE;
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

    const float4 projected_extrude = line.matrix * float4(dist / line.ratio, 0.0, 0.0);
    const float4 position = line.matrix * float4(pos + offset2 / line.ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * line.units_to_pixels);

    return {
        .position     = position,
        .width2       = float2(outset, inset),
        .normal       = v_normal,
        .gamma_scale  = extrude_length_without_perspective / extrude_length_with_perspective,
        .blur         = blur,
        .opacity      = opacity,
        .pattern_from = pattern_from,
        .pattern_to   = pattern_to,
        .linesofar    = linesofar,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const LinePatternUBO& line [[buffer(9)]],
                            device const LinePermutationUBO& permutation [[buffer(13)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    float2 pattern_tl_a = in.pattern_from.xy;
    float2 pattern_br_a = in.pattern_from.zw;
    float2 pattern_tl_b = in.pattern_to.xy;
    float2 pattern_br_b = in.pattern_to.zw;
    
    float pixelRatio = line.scale.x;
    float tileZoomRatio = line.scale.y;
    float fromScale = line.scale.z;
    float toScale = line.scale.w;
    
    float2 display_size_a = float2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    float2 display_size_b = float2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);
    
    float2 pattern_size_a = float2(display_size_a.x * fromScale / tileZoomRatio, display_size_a.y);
    float2 pattern_size_b = float2(display_size_b.x * toScale / tileZoomRatio, display_size_b.y);
    
    // Calculate the distance of the pixel from the line in pixels.
    float dist = length(in.normal) * in.width2.x;
    
    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (in.width2.y) or when fading out
    // (in.width2.x)
    float blur2 = (in.blur + 1.0 / DEVICE_PIXEL_RATIO) * in.gamma_scale;
    float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0);
    
    float x_a = glMod(in.linesofar / pattern_size_a.x, 1.0);
    float x_b = glMod(in.linesofar / pattern_size_b.x, 1.0);
    
    // in.normal.y is 0 at the midpoint of the line, -1 at the lower edge, 1 at the upper edge
    // we clamp the line width outset to be between 0 and half the pattern height plus padding (2.0)
    // to ensure we don't sample outside the designated symbol on the sprite sheet.
    // 0.5 is added to shift the component to be bounded between 0 and 1 for interpolation of
    // the texture coordinate
    float y_a = 0.5 + (in.normal.y * clamp(in.width2.x, 0.0, (pattern_size_a.y + 2.0) / 2.0) / pattern_size_a.y);
    float y_b = 0.5 + (in.normal.y * clamp(in.width2.x, 0.0, (pattern_size_b.y + 2.0) / 2.0) / pattern_size_b.y);
    float2 pos_a = mix(pattern_tl_a / line.texsize, pattern_br_a / line.texsize, float2(x_a, y_a));
    float2 pos_b = mix(pattern_tl_b / line.texsize, pattern_br_b / line.texsize, float2(x_b, y_b));

    float4 color = mix(image0.sample(image0_sampler, pos_a), image0.sample(image0_sampler, pos_b), line.fade);
    
    return half4(color * alpha * in.opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LineSDFShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static constexpr AttributeInfo attributes[] = {
        {0, gfx::AttributeDataType::Short2, 1, "a_pos_normal"},
        {1, gfx::AttributeDataType::UByte4, 1, "a_data"},
        {2, gfx::AttributeDataType::Float4, 1, "a_color"},
        {3, gfx::AttributeDataType::Float2, 1, "a_blur"},
        {4, gfx::AttributeDataType::Float2, 1, "a_opacity"},
        {5, gfx::AttributeDataType::Float2, 1, "a_gapwidth"},
        {6, gfx::AttributeDataType::Float2, 1, "a_offset"},
        {7, gfx::AttributeDataType::Float2, 1, "a_width"},
        {8, gfx::AttributeDataType::Float2, 1, "a_floorwidth"},
    };
    static constexpr UniformBlockInfo uniforms[] = {
        MLN_MTL_UNIFORM_BLOCK(9, true, true, LineSDFUBO),
        MLN_MTL_UNIFORM_BLOCK(10, true, false, LineSDFPropertiesUBO),
        MLN_MTL_UNIFORM_BLOCK(11, true, false, LineSDFInterpolationUBO),
        MLN_MTL_UNIFORM_BLOCK(12, true, true, LinePermutationUBO),
        MLN_MTL_UNIFORM_BLOCK(13, true, false, ExpressionInputsUBO),
    };
    static constexpr TextureInfo textures[] = {
        {0, "u_image"},
    };

    static constexpr auto source = R"(

struct VertexStage {
    short2 pos_normal [[attribute(0)]];
    uchar4 data [[attribute(1)]];
    float4 color [[attribute(2)]];
    float2 blur [[attribute(3)]];
    float2 opacity [[attribute(4)]];
    float2 gapwidth [[attribute(5)]];
    float2 offset [[attribute(6)]];
    float2 width [[attribute(7)]];
    float2 floorwidth [[attribute(8)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float4 color;
    float2 width2;
    float2 normal;
    float gamma_scale;
    float blur;
    float opacity;
    float2 tex_a;
    float2 tex_b;
    float floorwidth;
};

struct alignas(16) LineSDFUBO {
    float4x4 matrix;
    float2 units_to_pixels;
    float2 patternscale_a;
    float2 patternscale_b;
    float ratio;
    float device_pixel_ratio;
    float tex_y_a;
    float tex_y_b;
    float sdfgamma;
    float mix;
};

struct alignas(16) LineSDFPropertiesUBO {
    float4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    float pad1, pad2;
};

struct alignas(16) LineSDFInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float floorwidth_t;
    float pad1;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const LineSDFUBO& line [[buffer(9)]],
                                device const LineSDFPropertiesUBO& props [[buffer(10)]],
                                device const LineSDFInterpolationUBO& interp [[buffer(11)]],
                                device const LinePermutationUBO& permutation [[buffer(12)]],
                                device const ExpressionInputsUBO& expr [[buffer(13)]]) {

    const auto color      = colorFor(permutation.color,         props.color,        vertx.color,        interp.color_t,         expr);
    const auto blur       = valueFor(permutation.blur,          props.blur,         vertx.blur,         interp.blur_t,        expr);
    const auto opacity    = valueFor(permutation.opacity,       props.opacity,      vertx.opacity,      interp.opacity_t,       expr);
    const auto gapwidth   = valueFor(permutation.gapwidth,      props.gapwidth,     vertx.gapwidth,     interp.gapwidth_t,      expr) / 2;
    const auto offset     = valueFor(permutation.offset,        props.offset,       vertx.offset,       interp.offset_t,        expr) * -1;
    const auto width      = valueFor(permutation.width,         props.width,        vertx.width,        interp.width_t,         expr);
    const auto floorwidth = valueFor(permutation.floorwidth,    props.floorwidth,   vertx.floorwidth,   interp.floorwidth_t,    expr);

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / line.device_pixel_ratio / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
    const float a_direction = fmod(float(vertx.data.z), 4.0) - 1.0;
    float linesofar = (floor(vertx.data.z / 4.0) + vertx.data.w * 64.0) * LINE_DISTANCE_SCALE;
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

    const float4 projected_extrude = line.matrix * float4(dist / line.ratio, 0.0, 0.0);
    const float4 position = line.matrix * float4(pos + offset2 / line.ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * line.units_to_pixels);

    return {
        .position     = position,
        .color        = color,
        .width2       = float2(outset, inset),
        .normal       = v_normal,
        .gamma_scale  = extrude_length_without_perspective / extrude_length_with_perspective,
        .blur         = blur,
        .opacity      = opacity,
        .tex_a        = float2(linesofar * line.patternscale_a.x / floorwidth, (normal.y * line.patternscale_a.y + line.tex_y_a) * 2.0),
        .tex_b        = float2(linesofar * line.patternscale_b.x / floorwidth, (normal.y * line.patternscale_b.y + line.tex_y_b) * 2.0),
        .floorwidth   = floorwidth,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const LineSDFUBO& line [[buffer(9)]],
                            device const LinePermutationUBO& permutation [[buffer(12)]],
                            texture2d<float, access::sample> image0 [[texture(0)]],
                            sampler image0_sampler [[sampler(0)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    const float dist = length(in.normal) * in.width2.x;
    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (v_width2.y) or when fading out (v_width2.x)
    const float blur2 = (in.blur + 1.0 / line.device_pixel_ratio) * in.gamma_scale;
    float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0);

    float sdfdist_a = image0.sample(image0_sampler, in.tex_a).a;
    float sdfdist_b = image0.sample(image0_sampler, in.tex_b).a;
    float sdfdist = mix(sdfdist_a, sdfdist_b, line.mix);
    alpha *= smoothstep(0.5 - line.sdfgamma / in.floorwidth, 0.5 + line.sdfgamma / in.floorwidth, sdfdist);

    return half4(in.color * (alpha * in.opacity));
}
)";
};

//
// template <>
// struct ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal> {
//    static constexpr auto name = "LineGradientShader";
//    static constexpr auto vertexMainFunction = "vertexMain";
//    static constexpr auto fragmentMainFunction = "fragmentMain";
//
//    static constexpr AttributeInfo attributes[] = {
//        {0, gfx::AttributeDataType::Short2, 1, "a_pos_normal"},
//        {1, gfx::AttributeDataType::UByte4, 1, "a_data"},
//        {2, gfx::AttributeDataType::Float2, 1, "a_blur"},
//        {3, gfx::AttributeDataType::Float2, 1, "a_opacity"},
//        {4, gfx::AttributeDataType::Float2, 1, "a_gapwidth"},
//        {5, gfx::AttributeDataType::Float2, 1, "a_offset"},
//        {6, gfx::AttributeDataType::Float2, 1, "a_width"},
//    };
//    static constexpr UniformBlockInfo uniforms[] = {
//        MLN_MTL_UNIFORM_BLOCK(7, true, true, LineGradientUBO),
//        MLN_MTL_UNIFORM_BLOCK(8, true, false, LineGradientPropertiesUBO),
//        MLN_MTL_UNIFORM_BLOCK(9, true, false, LineGradientInterpolationUBO),
//        MLN_MTL_UNIFORM_BLOCK(10, true, true, LinePermutationUBO),
//        MLN_MTL_UNIFORM_BLOCK(11, true, false, ExpressionInputsUBO),
//    };
//    static constexpr TextureInfo textures[] = {
//        {0, "u_image"},
//    };
//
//    static constexpr auto source = R"(
//
// struct VertexStage {
//    short2 pos_normal [[attribute(0)]];
//    uchar4 data [[attribute(1)]];
//    float2 blur [[attribute(2)]];
//    float2 opacity [[attribute(3)]];
//    float2 gapwidth [[attribute(4)]];
//    float2 offset [[attribute(5)]];
//    float2 width [[attribute(6)]];
//};
//
// struct FragmentStage {
//    float4 position [[position, invariant]];
//    float2 width2;
//    float2 normal;
//    float gamma_scale;
//    float blur;
//    float opacity;
//    float lineprogress;
//};
//
//
//
// FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
//                                device const LineGradientUBO& line [[buffer(7)]],
//                                device const LineGradientPropertiesUBO& props [[buffer(8)]],
//                                device const LineGradientInterpolationUBO& interp [[buffer(9)]],
//                                device const LinePermutationUBO& permutation [[buffer(10)]],
//                                device const ExpressionInputsUBO& expr [[buffer(11)]]) {
//
//    const auto blur     = valueFor(permutation.blur,     props.blur,     vertx.blur,     interp.blur_t,     expr);
//    const auto opacity  = valueFor(permutation.opacity,  props.opacity,  vertx.opacity,  interp.opacity_t,  expr);
//    const auto gapwidth = valueFor(permutation.gapwidth, props.gapwidth, vertx.gapwidth, interp.gapwidth_t, expr) / 2;
//    const auto offset   = valueFor(permutation.offset,   props.offset,   vertx.offset,   interp.offset_t,   expr) *
//    -1; const auto width    = valueFor(permutation.width,    props.width,    vertx.width,    interp.width_t,    expr);
//
//    // the distance over which the line edge fades out.
//    // Retina devices need a smaller distance to avoid aliasing.
//    const float ANTIALIASING = 1.0 / line.device_pixel_ratio / 2.0;
//    const float LINE_DISTANCE_SCALE = 2.0;
//
//    const float2 a_extrude = float2(vertx.data.xy) - 128.0;
//    const float a_direction = fmod(float(vertx.data.z), 4.0) - 1.0;
//    float lineprogress = (floor(vertx.data.z / 4.0) + vertx.data.w * 64.0) * 2.0 / MAX_LINE_DISTANCE;
//    const float2 pos = floor(float2(vertx.pos_normal) * 0.5);
//
//    // x is 1 if it's a round cap, 0 otherwise
//    // y is 1 if the normal points up, and -1 if it points down
//    // We store these in the least significant bit of a_pos_normal
//    const float2 normal = float2(vertx.pos_normal) - 2.0 * pos;
//    const float2 v_normal = float2(normal.x, normal.y * 2.0 - 1.0);
//
//    const float halfwidth = width / 2.0;
//    const float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
//    const float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 :
//    ANTIALIASING);
//
//    // Scale the extrusion vector down to a normal and then up by the line width of this vertex.
//    const float2 dist = outset * a_extrude * LINE_NORMAL_SCALE;
//
//    // Calculate the offset when drawing a line that is to the side of the actual line.
//    // We do this by creating a vector that points towards the extrude, but rotate
//    // it when we're drawing round end points (a_direction = -1 or 1) since their
//    // extrude vector points in another direction.
//    const float u = 0.5 * a_direction;
//    const float t = 1.0 - abs(u);
//    const float2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * v_normal.y * float2x2(t, -u, u, t);
//
//    const float4 projected_extrude = line.matrix * float4(dist / line.ratio, 0.0, 0.0);
//    const float4 position = line.matrix * float4(pos + offset2 / line.ratio, 0.0, 1.0) + projected_extrude;
//
//    // calculate how much the perspective view squishes or stretches the extrude
//    const float extrude_length_without_perspective = length(dist);
//    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * line.units_to_pixels);
//
//    return {
//        .position     = position,
//        .width2       = float2(outset, inset),
//        .normal       = v_normal,
//        .gamma_scale  = extrude_length_without_perspective / extrude_length_with_perspective,
//        .blur         = blur,
//        .opacity      = opacity,
//        .lineprogress = lineprogress,
//    };
//}
//
// half4 fragment fragmentMain(FragmentStage in [[stage_in]],
//                            device const LineSDFUBO& line [[buffer(9)]],
//                            device const LinePermutationUBO& permutation [[buffer(12)]],
//                            texture2d<float, access::sample> image0 [[texture(0)]],
//                            sampler image0_sampler [[sampler(0)]]) {
//    if (permutation.overdrawInspector) {
//        return half4(1.0);
//    }
//
//    const float dist = length(in.normal) * in.width2.x;
//    // Calculate the antialiasing fade factor. This is either when fading in the
//    // line in case of an offset line (v_width2.y) or when fading out (v_width2.x)
//    const float blur2 = (in.blur + 1.0 / line.device_pixel_ratio) * in.gamma_scale;
//    const float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0);
//
//    float4 color = image0.sample(image0_sampler, float2(in.v_lineprogress, 0.5));
//
//    return half4(color * (alpha * in.opacity));
//}
//)";
//};

} // namespace shaders
} // namespace mbgl
