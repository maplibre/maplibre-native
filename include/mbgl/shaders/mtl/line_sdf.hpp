// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal> {


    static const ReflectionData reflectionData;
    static constexpr const char* sourceData = R"(struct VertexStage {
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
})";
    static std::string source() {
        using Ty = ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Metal>;
        return Ty::sourceData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
