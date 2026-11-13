#pragma once

#include <mln/shaders/shader_source.hpp>
#include <mln/shaders/vulkan/shader_program.hpp>

namespace mln {
namespace shaders {

/// Shared by both draws of the ground ambient occlusion. The blocks here must stay layout-identical to the
/// C++ structs in include/mln/shaders/fill_extrusion_ground_occlusion_layer_ubo.hpp.
constexpr auto fillExtrusionGroundOcclusionShaderPrelude = R"(

#define idFillExtrusionGroundOcclusionDrawableUBO           idDrawableReservedVertexOnlyUBO
#define idFillExtrusionGroundOcclusionPropsUBO              layerUBOStartId
#define idFillExtrusionGroundOcclusionInstancedDrawableUBO  drawableSSBOStartId

layout(set = LAYER_SET_INDEX, binding = idFillExtrusionGroundOcclusionPropsUBO) uniform FillExtrusionGroundOcclusionPropsUBO {
    float intensity;
    float base;
    float height;
    float pad1;
} props;

)";

//
// Ground ambient occlusion around the footprints (instanced, one quad per outline edge)
//
// Each edge draws a capsule around itself: a quad extended by the radius past both ends and to
// both sides, fading out with the distance to the edge. Overlapping quads are combined with their
// maximum, so the result fades with the distance to the nearest wall. That takes two draws of the
// same quads: the first keeps the minimum of 1 - occlusion in the framebuffer's alpha, which is 1
// under the opaque map, and the second darkens the color by it and resets the alpha to 1, so any
// further overlapping quads leave it alone. See include/mln/shaders/mtl/fill_extrusion_ground_occlusion.hpp.

constexpr auto fillExtrusionGroundOcclusionVertexSource = R"(

// Static unit quad: x in {0,1} selects p1 or p2, y in {0,1} the inner or the outer side.
layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_base)
layout(location = 3) in vec2 in_base;
#endif

#if !defined(HAS_UNIFORM_u_height)
layout(location = 4) in vec2 in_height;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillExtrusionGroundOcclusionDrawableUBO {
    mat4 matrix;
    // Interpolations
    float base_t;
    float height_t;
    float meters_to_tile_units;
    float radius;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillExtrusionGroundOcclusionDrawableUBO) readonly buffer FillExtrusionGroundOcclusionDrawableUBOVector {
    FillExtrusionGroundOcclusionDrawableUBO drawable_ubo[];
} drawableVector;

// The outline of the bucket, as the instanced fill-extrusion walls read it
struct OutlineInstance {
    int pos;
    uint decimals_ed;
};

layout(std430, set = DRAWABLE_UBO_SET_INDEX, binding = idFillExtrusionGroundOcclusionInstancedDrawableUBO) readonly buffer FillExtrusionGroundOcclusionInstanceVector {
    OutlineInstance instance[];
} instanceVector;

// Signed distance from the wall, negative towards the inside, and the distances before the start
// of the edge and past its end, negative along it. All in units of the radius.
layout(location = 0) out vec3 frag_distance;
// How much the occlusion applies, 0 for parts that do not stand on the ground
layout(location = 1) out float frag_strength;

void main() {
    frag_distance = vec3(0.0);
    frag_strength = 0.0;

    const vec2 decimalsEd = unpack_uint(instanceVector.instance[gl_InstanceIndex].decimals_ed);
    // The low bit of decimals_ed.x marks the last vertex of each ring, which starts no edge
    if (mod(decimalsEd.x, 2.0) > 0.0) {
        gl_Position = vec4(0.0);
        return;
    }

    const FillExtrusionGroundOcclusionDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_base)
    const float base = max(props.base, 0.0);
#else
    const float base = max(unpack_mix_float(in_base, drawable.base_t), 0.0);
#endif

#if defined(HAS_UNIFORM_u_height)
    const float height = max(props.height, 0.0);
#else
    const float height = max(unpack_mix_float(in_height, drawable.height_t), 0.0);
#endif

    const vec2 nextDecimalsEd = unpack_uint(instanceVector.instance[gl_InstanceIndex + 1].decimals_ed);
    const vec2 p1 = unpack_int(instanceVector.instance[gl_InstanceIndex].pos) + unpack_float(floor(decimalsEd.x / 2.0)) / 128.0;
    const vec2 p2 = unpack_int(instanceVector.instance[gl_InstanceIndex + 1].pos) + unpack_float(floor(nextDecimalsEd.x / 2.0)) / 128.0;

    const float radius = drawable.radius * drawable.meters_to_tile_units;
    const float edgeLength = length(p2 - p1);
    // Flat footprints have no walls to occlude the ground
    if (radius <= 0.0 || edgeLength == 0.0 || height <= base) {
        gl_Position = vec4(0.0);
        return;
    }
    const vec2 along = (p2 - p1) / edgeLength;
    // The capsule covers both sides of the edge, so the winding does not matter. The half under
    // the building is mostly hidden by the depth test, but it covers the ground that shows through
    // cracks between the building's own triangles.
    const vec2 outward = vec2(along.y, -along.x);

    const bool atEnd = in_position.x != 0;
    const float outer = in_position.y != 0 ? 1.0 : -1.0;
    const float u = atEnd ? edgeLength / radius + 1.0 : -1.0;

    gl_Position = drawable.matrix * vec4(p1 + (along * u + outward * outer) * radius, 0.0, 1.0);
    applySurfaceTransform();

    frag_distance = vec3(outer, -u, u - edgeLength / radius);
    frag_strength = ambientOcclusionFalloff(base / drawable.radius);
}
)";

template <>
struct ShaderSource<BuiltIn::FillExtrusionGroundOcclusionShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillExtrusionGroundOcclusionShader";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillExtrusionGroundOcclusionShaderPrelude;
    static constexpr auto vertex = fillExtrusionGroundOcclusionVertexSource;

    // First draw: the minimum of 1 - occlusion goes into the alpha
    static constexpr auto fragment = R"(

layout(location = 0) in vec3 frag_distance;
layout(location = 1) in float frag_strength;

layout(location = 0) out vec4 out_color;

void main() {
    const float occlusion = props.intensity * frag_strength *
                            ambientOcclusionFalloff(length(vec3(abs(frag_distance.x), max(frag_distance.yz, 0.0))));
    out_color = vec4(0.0, 0.0, 0.0, 1.0 - occlusion);
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionGroundOcclusionResolveShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillExtrusionGroundOcclusionResolveShader";

    static const std::array<AttributeInfo, 1> attributes;
    static const std::array<AttributeInfo, 4> instanceAttributes;
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillExtrusionGroundOcclusionShaderPrelude;
    static constexpr auto vertex = fillExtrusionGroundOcclusionVertexSource;

    // Second draw, blended (One, DstAlpha): darkens the color by the alpha and resets it to 1
    static constexpr auto fragment = R"(

layout(location = 0) in vec3 frag_distance;
layout(location = 1) in float frag_strength;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(0.0, 0.0, 0.0, 1.0);
}
)";
};

} // namespace shaders
} // namespace mln
