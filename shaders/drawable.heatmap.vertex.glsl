layout (location = 0) in vec2 a_pos;
out vec2 v_extrude;

layout (std140) uniform HeatmapDrawableUBO {
    highp mat4 u_matrix;
    highp float u_extrude_scale;
    lowp float drawable_pad1;
    lowp vec2 drawable_pad2;
};

layout (std140) uniform HeatmapEvaluatedPropsUBO {
    highp float u_weight;
    highp float u_radius;
    highp float u_intensity;
    lowp float props_pad1;
};

layout (std140) uniform HeatmapInterpolateUBO {
    lowp float u_weight_t;
    lowp float u_radius_t;
    lowp vec2 interp_pad1;
};

#pragma mapbox: define highp float weight
#pragma mapbox: define mediump float radius

// Effective "0" in the kernel density texture to adjust the kernel size to;
// this empirically chosen number minimizes artifacts on overlapping kernels
// for typical heatmap cases (assuming clustered source)
const highp float ZERO = 1.0 / 255.0 / 16.0;

// Gaussian kernel coefficient: 1 / sqrt(2 * PI)
#define GAUSS_COEF 0.3989422804014327

void main(void) {
    #pragma mapbox: initialize highp float weight
    #pragma mapbox: initialize mediump float radius

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
