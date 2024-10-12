#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "HeatmapShader";

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

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

layout(set = 0, binding = 1) uniform HeatmapDrawableUBO {
    mat4 matrix;
    float extrude_scale;
    float pad1;
    vec2 pad2;
} drawable;

layout(set = 0, binding = 2) uniform HeatmapEvaluatedPropsUBO {
    float weight;
    float radius;
    float intensity;
    float pad1;
} props;

layout(set = 0, binding = 3) uniform HeatmapInterpolateUBO {
    float weight_t;
    float radius_t;
    vec2 pad1;
} interp;

layout(location = 0) out float frag_weight;
layout(location = 1) out lowp vec2 frag_extrude;

void main() {

#if defined(HAS_UNIFORM_u_weight)
    const float weight = props.weight;
#else
    const float weight = unpack_mix_float(in_weight, interp.weight_t);
#endif

#if defined(HAS_UNIFORM_u_radius)
    const float radius = props.radius;
#else
    const float radius = unpack_mix_float(in_radius, interp.radius_t);
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
    gl_Position.y *= -1.0;

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

layout(set = 0, binding = 2) uniform HeatmapEvaluatedPropsUBO {
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

template <>
struct ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "HeatmapTextureShader";

    static const std::array<UniformBlockInfo, 2> uniforms;
    static const std::array<AttributeInfo, 1> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 2> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

layout(set = 0, binding = 1) uniform HeatmapTexturePropsUBO {
    mat4 matrix;
    float opacity;
    float pad1, pad2, pad3;
} props;

layout(location = 0) out vec2 frag_position;

void main() {

    gl_Position = props.matrix * vec4(in_position * global.world_size, 0, 1);
    gl_Position.y *= -1.0;

    frag_position = in_position;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position;
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform HeatmapTexturePropsUBO {
    mat4 matrix;
    float opacity;
    float pad1, pad2, pad3;
} props;

layout(set = 1, binding = 0) uniform sampler2D image_sampler;
layout(set = 1, binding = 1) uniform sampler2D color_ramp_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const float t = texture(image_sampler, frag_position).r;
    const vec4 color = texture(color_ramp_sampler, vec2(t, 0.5));
    out_color = color * props.opacity;
}
)";
};

} // namespace shaders
} // namespace mbgl
