#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) ClipUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ std::uint32_t stencil_ref;
    /* 68 */ float pad1;
    /* 72 */ float pad2;
    /* 76 */ float pad3;
    /* 80 */
};
static_assert(sizeof(ClipUBO) == 5 * 16);

#define CLIPPING_MASK_SHADER_PRELUDE R"(

#include <metal_stdlib>
using namespace metal;

struct alignas(16) ClipUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ uint32_t stencil_ref;
    /* 68 */ float pad1;
    /* 72 */ float pad2;
    /* 76 */ float pad3;
    /* 80 */
};
static_assert(sizeof(ClipUBO) == 5 * 16, "wrong size");

enum {
    idClippingMaskUBO = globalUBOCount,
    clippingMaskUBOCount
};

)"

template <>
struct ShaderSource<BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal> {
    static constexpr auto name = "ClippingMaskProgram";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto source = CLIPPING_MASK_SHADER_PRELUDE R"(
struct VertexStage {
    short2 position [[attribute(clippingMaskUBOCount + 0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

struct FragmentResult {
    // color output is only needed because we're using implicit stencil writes
    half4 color [[color(0)]];
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const ClipUBO& clipUBO [[buffer(idClippingMaskUBO)]]) {
    return { clipUBO.matrix * float4(float2(in.position.xy), 0, 1) };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]]) {
    return half4(1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl
