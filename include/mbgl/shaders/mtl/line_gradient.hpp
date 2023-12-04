#pragma once

#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/common.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "LineGradientShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";
    static constexpr auto hasPermutations = false;

    static const std::array<AttributeInfo, 7> attributes;
    static const std::array<UniformBlockInfo, 5> uniforms;
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
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 width2;
    float2 gapwidth;
    float2 normal;
    float gamma_scale;
    float blur;
    float opacity;
    float lineprogress;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const LineGradientUBO& line [[buffer(7)]],
                                device const LineGradientPropertiesUBO& props [[buffer(8)]],
                                device const LineGradientInterpolationUBO& interp [[buffer(9)]],
                                device const LinePermutationUBO& permutation [[buffer(10)]],
                                device const ExpressionInputsUBO& expr [[buffer(11)]]) {

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

    const float4 projected_extrude = line.matrix * float4(dist / line.ratio, 0.0, 0.0);
    const float4 position = line.matrix * float4(pos + offset2 / line.ratio, 0.0, 1.0) + projected_extrude;

    // calculate how much the perspective view squishes or stretches the extrude
    const float extrude_length_without_perspective = length(dist);
    const float extrude_length_with_perspective = length(projected_extrude.xy / position.w * line.units_to_pixels);

    return {
        .position     = position,
        .width2       = float2(outset, inset),
        .gapwidth     = gapwidth,
        .normal       = v_normal,
        .gamma_scale  = extrude_length_without_perspective / extrude_length_with_perspective,
        .blur         = blur,
        .opacity      = opacity,
        .lineprogress = v_lineprogress,
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const LineGradientUBO& line [[buffer(7)]],
                            device const LinePermutationUBO& permutation [[buffer(10)]],
                            texture2d<float, access::sample> gradientTexture [[texture(0)]]) {
    if (permutation.overdrawInspector) {
        return half4(1.0);
    }

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(in.normal) * in.width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (v_width2.y) or when fading out (v_width2.x)
    const float blur2 = (in.blur + 1.0 / line.device_pixel_ratio) * in.gamma_scale;
    const float alpha = clamp(min(dist - (in.width2.y - blur2), in.width2.x - dist) / blur2, 0.0, 1.0);

    // For gradient lines, v_lineprogress is the ratio along the entire line,
    // scaled to [0, 2^15), and the gradient ramp is stored in a texture.
    constexpr sampler sampler2d(coord::normalized, filter::linear);
    const float4 color = gradientTexture.sample(sampler2d, float2(in.lineprogress, 0.5));

    return half4(color * (alpha * in.opacity));
}
)";
};

} // namespace shaders
} // namespace mbgl
