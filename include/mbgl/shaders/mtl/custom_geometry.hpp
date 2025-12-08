#pragma once

#include <mbgl/shaders/custom_geometry_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto customGeometryShaderPrelude = R"(

enum {
    idCustomGeometryDrawableUBO = drawableReservedUBOCount,
    customGeometryUBOCount
};

struct alignas(16) CustomGeometryDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float4 color;
    /* 80 */
};
static_assert(sizeof(CustomGeometryDrawableUBO) == 5 * 16, "wrong size");

)";

template <>
struct ShaderSource<BuiltIn::CustomGeometryShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "CustomGeometryShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = customGeometryShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    float3 position [[attribute(customGeometryUBOCount + 0)]];
    float2 uv [[attribute(customGeometryUBOCount + 1)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
    float2 uv;
};

FragmentStage vertex vertexMain(thread const VertexStage vertx [[stage_in]],
                                device const CustomGeometryDrawableUBO& drawable [[buffer(idCustomGeometryDrawableUBO)]]) {

    return {
        .position = drawable.matrix * float4(vertx.position, 1.0),
        .uv = vertx.uv
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]],
                            device const CustomGeometryDrawableUBO& drawable [[buffer(idCustomGeometryDrawableUBO)]],
                            texture2d<float, access::sample> colorTexture [[texture(0)]]) {
    constexpr sampler sampler2d(coord::normalized, filter::linear);
    const float4 color = colorTexture.sample(sampler2d, in.uv) * drawable.color;

    return half4(color);
}
)";
};

} // namespace shaders
} // namespace mbgl
