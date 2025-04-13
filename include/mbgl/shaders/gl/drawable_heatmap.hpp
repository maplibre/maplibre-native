// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HeatmapShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "HeatmapShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
out vec2 v_extrude;

layout (std140) uniform HeatmapDrawableUBO {
    highp mat4 u_matrix;
    highp float u_extrude_scale;
    // Interpolations
    lowp float u_weight_t;
    lowp float u_radius_t;
    lowp float drawable_pad1;
};

layout (std140) uniform HeatmapEvaluatedPropsUBO {
    highp float u_weight;
    highp float u_radius;
    highp float u_intensity;
    lowp float props_pad1;
};

#ifndef HAS_UNIFORM_u_weight
layout (location = 1) in highp vec2 a_weight;
out highp float weight;
#endif
#ifndef HAS_UNIFORM_u_radius
layout (location = 2) in mediump vec2 a_radius;
#endif

// Effective "0" in the kernel density texture to adjust the kernel size to;
// this empirically chosen number minimizes artifacts on overlapping kernels
// for typical heatmap cases (assuming clustered source)
const highp float ZERO = 1.0 / 255.0 / 16.0;

// Gaussian kernel coefficient: 1 / sqrt(2 * PI)
#define GAUSS_COEF 0.3989422804014327

void main(void) {
    #ifndef HAS_UNIFORM_u_weight
weight = unpack_mix_vec2(a_weight, u_weight_t);
#else
highp float weight = u_weight;
#endif
    #ifndef HAS_UNIFORM_u_radius
mediump float radius = unpack_mix_vec2(a_radius, u_radius_t);
#else
mediump float radius = u_radius;
#endif

    // unencode the extrusion vector that we snuck into the a_pos vector
    vec2 unscaled_extrude = vec2(mod(a_pos, 2.0) * 2.0 - 1.0);

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
    float S = sqrt(-2.0 * log(ZERO / weight / u_intensity / GAUSS_COEF)) / 3.0;

    // Pass the varying in units of radius
    v_extrude = S * unscaled_extrude;

    // Scale by radius and the zoom-based scale factor to produce actual
    // mesh position
    vec2 extrude = v_extrude * radius * u_extrude_scale;

    // multiply a_pos by 0.5, since we had it * 2 in order to sneak
    // in extrusion data
    vec4 pos = vec4(floor(a_pos * 0.5) + extrude, 0, 1);

    gl_Position = u_matrix * pos;
}
)";
    static constexpr const char* fragment = R"(in vec2 v_extrude;

layout (std140) uniform HeatmapEvaluatedPropsUBO {
    highp float u_weight;
    highp float u_radius;
    highp float u_intensity;
    lowp float props_pad1;
};

#ifndef HAS_UNIFORM_u_weight
in highp float weight;
#endif

// Gaussian kernel coefficient: 1 / sqrt(2 * PI)
#define GAUSS_COEF 0.3989422804014327

void main() {
    #ifdef HAS_UNIFORM_u_weight
highp float weight = u_weight;
#endif

    // Kernel density estimation with a Gaussian kernel of size 5x5
    float d = -0.5 * 3.0 * 3.0 * dot(v_extrude, v_extrude);
    float val = weight * u_intensity * GAUSS_COEF * exp(d);

    fragColor = vec4(val, 1.0, 1.0, 1.0);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
