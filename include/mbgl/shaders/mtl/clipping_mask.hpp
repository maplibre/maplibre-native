// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/clipping_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal> {
    static const ReflectionData reflectionData;
    static constexpr const char* sourceData = R"(#include <metal_stdlib>
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
};

struct FragmentResult {
    // color output is only needed because we're using implicit stencil writes
    half4 color [[color(0)]];
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const ClipUBO& clipUBO [[buffer(1)]]) {
    return { clipUBO.matrix * float4(float2(in.position.xy), 0, 1) };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]]) {
    return half4(1.0);
}
)";
    static std::string source() {
        using Ty = ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal>;
        return Ty::sourceData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
