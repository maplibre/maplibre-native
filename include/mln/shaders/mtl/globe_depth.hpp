#pragma once

#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/mtl/shader_program.hpp>

namespace mln {
namespace shaders {

enum {
    globeDepthUBOCount = drawableReservedUBOCount
};

constexpr auto globeDepthShaderPrelude = R"(

enum {
    globeDepthUBOCount = drawableReservedUBOCount
};

)";

/// Writes the planet surface into the depth buffer so 3D geometry behind the horizon is hidden.
template <>
struct ShaderSource<BuiltIn::GlobeDepthShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "GlobeDepthShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = globeDepthShaderPrelude;
    static constexpr auto source = R"(

struct VertexStage {
    short2 position [[attribute(0)]];
};

struct FragmentStage {
    float4 position [[position, invariant]];
};

FragmentStage vertex vertexMain(VertexStage in [[stage_in]],
                                device const uint32_t& uboIndex [[buffer(idGlobalUBOIndex)]],
                                device const ProjectionUBO* projectionVector [[buffer(idProjectionUBO)]]) {
    return {
        .position = projectTileFor3D(float2(in.position.xy), 0.0, projectionVector[uboIndex])
    };
}

half4 fragment fragmentMain(FragmentStage in [[stage_in]]) {
    return half4(0.0);
}
)";
};

} // namespace shaders
} // namespace mln
