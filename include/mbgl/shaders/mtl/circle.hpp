#pragma once

#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "CircleShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static constexpr AttributeInfo attributes[] = {
        {0, "a_pos", gfx::AttributeDataType::Float3, 1},
    };
    static constexpr UniformBlockInfo uniforms[] = {
        { 8, sizeof(CircleDrawableUBO), true, false, "CircleDrawableUBO" },
        { 9, sizeof(CirclePaintParamsUBO), true, true, "CirclePaintParamsUBO" },
        { 10, sizeof(CircleEvaluatedPropsUBO), true, true, "CircleEvaluatedPropsUBO" },
    };

    static constexpr auto source = R"(
#include <metal_stdlib>
using namespace metal;

struct alignas(16) CircleDrawableUBO {
    float4x4 matrix;
    float2 extrude_scale;
    float2 padding;
};
struct alignas(16) CirclePaintParamsUBO {
    float camera_to_center_distance;
    float device_pixel_ratio;
    float pad1,pad2;
};
struct alignas(16) CircleEvaluatedPropsUBO {
    float4 color;
    float4 stroke_color;
    float radius;
    float blur;
    float opacity;
    float stroke_width;
    float stroke_opacity;
    int scale_with_map;
    int pitch_with_map;
    float padding;
};

struct v2f {
    float4 position [[position]];
};

v2f vertex vertexMain(uint vertexId [[vertex_id]],
                      device const short2* positions [[buffer(0)]],
                      device const float4* a_color [[buffer(1)]],
                      device const float2* a_radius [[buffer(2)]],
                      device const float2* a_blur [[buffer(3)]],
                      device const float2* a_opacity [[buffer(4)]],
                      device const float4* a_stroke_color [[buffer(5)]],
                      device const float2* a_stroke_width [[buffer(6)]],
                      device const float2* a_stroke_opacity [[buffer(7)]],
                      device const CircleDrawableUBO& drawableUBO [[buffer(8)]],
                      device const CirclePaintParamsUBO& paramsUBO [[buffer(9)]],
                      device const CircleEvaluatedPropsUBO& propsUBO [[buffer(10)]]) {
    return { drawableUBO.matrix * float4(positions[vertexId].x, positions[vertexId].y, 0.0, 1.0) };
}

half4 fragment fragmentMain(v2f in [[stage_in]],
                            device const CirclePaintParamsUBO& paramsUBO [[buffer(2)]],
                            device const CircleEvaluatedPropsUBO& propsUBO [[buffer(3)]]) {
#ifdef OVERDRAW_INSPECTOR
    return half4(1.0);
#else
    return half4(propsUBO.color) * propsUBO.opacity;
#endif
}
)";
};


} // namespace shaders
} // namespace mbgl
