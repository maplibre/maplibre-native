#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto heatmapShaderPrelude = R"(

#define idHeatmapDrawableUBO        idDrawableReservedVertexOnlyUBO
#define idHeatmapEvaluatedPropsUBO  layerUBOStartId

)";

template <>
struct ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "HeatmapShader";

    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = heatmapShaderPrelude;
    static constexpr auto vertex = R"(

// Effective "0" in the kernel density texture to adjust the kernel size to;
// this empirically chosen number minimizes artifacts on overlapping kernels
// for typical heatmap cases (assuming clustered source)
const float ZERO = 1.0 / 255.0 / 16.0;

// Gaussian kernel coefficient: 1 / sqrt(2 * PI)
#define GAUSS_COEF 0.3989422804014327

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_weight)
layout(location = 1) in vec2 in_weight;
#endif

#if !defined(HAS_UNIFORM_u_radius)
layout(location = 2) in vec2 in_radius;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct HeatmapDrawableUBO {
    mat4 matrix;
    float extrude_scale;
    // Interpolations
    float weight_t;
    float radius_t;
    float pad1;
};

layout(std140, set = LAYER_SET_INDEX, binding = idHeatmapDrawableUBO) readonly buffer HeatmapDrawableUBOVector {
    HeatmapDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = LAYER_SET_INDEX, binding = idHeatmapEvaluatedPropsUBO) uniform HeatmapEvaluatedPropsUBO {
    float weight;
    float radius;
    float intensity;
    float pad1;
} props;

layout(location = 0) out float frag_weight;
layout(location = 1) out lowp vec2 frag_extrude;

void main() {
    const HeatmapDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_weight)
    const float weight = props.weight;
#else
    const float weight = unpack_mix_float(in_weight, drawable.weight_t);
#endif

#if defined(HAS_UNIFORM_u_radius)
    const float radius = props.radius;
#else
    const float radius = unpack_mix_float(in_radius, drawable.radius_t);
#endif

    // unencode the extrusion vector that we snuck into the a_pos vector
    const vec2 unscaled_extrude = mod(in_position, 2.0) * 2.0 - 1.0;

    // This 'extrude' comes in ranging from [-1, -1], to [1, 1].  We'll use
    // it to produce the vertices of a square mesh framing the point feature
    // we're adding to the kernel density texture.  We'll also pass it as
    // a varying, so that the fragment shader can determine the distance of
    // each fragment from the point feature.
    // Before we do so, we need to scale it up sufficiently so that the
    // kernel falls effectively to zero at the edge of the mesh.
    // That is, we want to know S such that
    // weight * u_intensity * GAUSS_COEF * exp(-0.5 * 3.0^2 * S^2) == ZERO
    // Which solves to:
    // S = sqrt(-2.0 * log(ZERO / (weight * u_intensity * GAUSS_COEF))) / 3.0
    const float S = sqrt(-2.0 * log(ZERO / weight / props.intensity / GAUSS_COEF)) / 3.0;

    // Pass the varying in units of radius
    const vec2 extrude = S * unscaled_extrude;

    // Scale by radius and the zoom-based scale factor to produce actual
    // mesh position
    const vec2 scaled_extrude = extrude * radius * drawable.extrude_scale;

    // multiply a_pos by 0.5, since we had it * 2 in order to sneak
    // in extrusion data
    gl_Position = drawable.matrix * vec4(floor(in_position * 0.5) + scaled_extrude, 0, 1);
    applySurfaceTransform();

    frag_weight = weight;
    frag_extrude = extrude;
}
)";

    static constexpr auto fragment = R"(

// Gaussian kernel coefficient: 1 / sqrt(2 * PI)
#define GAUSS_COEF 0.3989422804014327

layout(location = 0) in float frag_weight;
layout(location = 1) in lowp vec2 frag_extrude;

layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idHeatmapEvaluatedPropsUBO) uniform HeatmapEvaluatedPropsUBO {
    float weight;
    float radius;
    float intensity;
    float pad1;
} props;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    // Kernel density estimation with a Gaussian kernel of size 5x5
    const float d = -0.5 * 3.0 * 3.0 * dot(frag_extrude, frag_extrude);
    const float val = frag_weight * props.intensity * GAUSS_COEF * exp(d);

    out_color = vec4(val, 1.0, 1.0, 1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl
