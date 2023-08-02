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

    static constexpr AttributeInfo attributes[] = {
        {0, gfx::AttributeDataType::Short2, 1, "a_pos_normal"},
        {1, gfx::AttributeDataType::UByte4, 1, "a_data"},
        {2, gfx::AttributeDataType::Float4, 1, "a_color"},
        {3, gfx::AttributeDataType::Float2, 1, "a_blur"},
        {4, gfx::AttributeDataType::Float2, 1, "a_opacity"},
        {5, gfx::AttributeDataType::Float2, 1, "a_gapwidth"},
        {6, gfx::AttributeDataType::Float2, 1, "a_offset"},
        {7, gfx::AttributeDataType::Float2, 1, "a_width"},
    };
    static constexpr UniformBlockInfo uniforms[] = {
        MLN_MTL_UNIFORM_BLOCK(8, true, true, LineUBO),
        MLN_MTL_UNIFORM_BLOCK(9, true, false, LinePropertiesUBO),
        MLN_MTL_UNIFORM_BLOCK(10, true, false, LineInterpolationUBO),
        MLN_MTL_UNIFORM_BLOCK(11, true, true, LinePermutationUBO),
        MLN_MTL_UNIFORM_BLOCK(12, true, false, ExpressionInputsUBO),
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

struct alignas(16) LineUBO {
    float4x4 matrix;
    float2 units_to_pixels;
    float ratio;
    float device_pixel_ratio;
};

struct alignas(16) LinePropertiesUBO {
    float4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float pad1, pad2, pad3;
};

struct alignas(16) LineInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1, pad2;
};

struct alignas(16) LinePermutationUBO {
    Attribute color;
    Attribute blur;
    Attribute opacity;
    Attribute gapwidth;
    Attribute offset;
    Attribute width;
    Attribute floorwidth;
    Attribute pattern_from;
    Attribute pattern_to;
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
    float pad4;
};

// the maximum allowed miter limit is 2.0 at the moment. the extrude normal is
// stored in a byte (-128..127). we scale regular normals up to length 63, but
// there are also "special" normals that have a bigger length (of up to 126 in
// this case).
constant float scale = 1.0 / (127 / 2);

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
    const float2 dist = outset * a_extrude * scale;

    // Calculate the offset when drawing a line that is to the side of the actual line.
    // We do this by creating a vector that points towards the extrude, but rotate
    // it when we're drawing round end points (a_direction = -1 or 1) since their
    // extrude vector points in another direction.
    const float u = 0.5 * a_direction;
    const float t = 1.0 - abs(u);
    const float2 offset2 = offset * a_extrude * scale * v_normal.y * float2x2(t, -u, u, t);

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

} // namespace shaders
} // namespace mbgl
