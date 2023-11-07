#include <metal_stdlib>
using namespace metal;

struct alignas(16) ClipUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ uint32_t stencil_ref;
    /* 68 */ uint32_t pad1, pad2, pad3;
    /* 80 */
};
static_assert(sizeof(ClipUBO) == 5 * 16, "unexpected padding");

struct VertexStage {
    short2 position [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    uint8_t stencil_ref;
};

struct FragmentResult {
    half4 color [[color(0)]];
    // target is `..._stencil8`, but using `uint8_t` here causes a compile error
    uint16_t stencil_ref [[stencil]];
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                uint16_t instanceID [[instance_id]],
                                device const ClipUBO* clipUBOs [[buffer(1)]]) {
    device const ClipUBO& clipUBO = clipUBOs[instanceID];
    return {
        .position = clipUBO.matrix * float4(float2(in.position.xy), 0, 1),
        .stencil_ref = static_cast<uint8_t>(clipUBO.stencil_ref),
    };
}

FragmentResult fragment fragmentMain(FragmentStage in [[stage_in]]) {
    return {
        .color = half4(1.0),
        .stencil_ref = static_cast<uint16_t>(in.stencil_ref),
    };
}
