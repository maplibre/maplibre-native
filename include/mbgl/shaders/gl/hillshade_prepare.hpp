// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "HillshadePrepareShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;

layout (std140) uniform HillshadePrepareDrawableUBO {
    highp mat4 u_matrix;
};

layout (std140) uniform HillshadePrepareTilePropsUBO {
    highp vec4 u_unpack;
    highp vec2 u_dimension;
    highp float u_zoom;
    highp float u_maxzoom;
};

out vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);

    // u_dimension is the DEM buffer stride (interior + 2 * border).
    // The buffer has `border = 3` so stride = dim + 6.
    //
    // Render target is (dim+3) × (dim+3). Output pixel i (i ∈ [0, dim+2])
    // computes the Sobel-derived slope centred on buffer texel index i+2
    // (the first interior cell at i=0 through the second-east-border
    // cell at i=dim). The east-border cells hold the eastern neighbour's
    // first two interior columns, so the output pixel at i=dim of tile A
    // and the output pixel at i=0 of tile B reference identical source
    // pixels and produce identical slope values — which is what makes
    // the rendered hillshade seamless at the tile boundary.
    //
    // Quad corners have a_texture_pos ∈ {0, EXTENT}². At the rendered
    // framebuffer corner (0, 0) we want v_pos at the texel-2 centre,
    // i.e. 2.5 / stride. At the opposite corner (EXTENT, EXTENT) we
    // want v_pos at the texel-(dim+2) centre, i.e. (dim + 2.5) / stride.
    // Given output pixel i sits at framebuffer-coord (i + 0.5)/(dim+1),
    // the linear interpolation that lands those endpoints is:
    //
    //   v_pos = (a_texture_pos / EXTENT) * (dim + 1)/stride + 2/stride
    //
    // Or, in terms of u_dimension = stride = dim + 4:
    float scale = (u_dimension.x - 3.0) / u_dimension.x;
    v_pos = (a_texture_pos / 8192.0) * scale + 2.0 / u_dimension;
}
)";
    static constexpr const char* fragment = R"(#ifdef GL_ES
precision highp float;
#endif

in vec2 v_pos;
uniform sampler2D u_image;
layout (std140) uniform HillshadePrepareTilePropsUBO {
    highp vec4 u_unpack;
    highp vec2 u_dimension;
    highp float u_zoom;
    highp float u_maxzoom;
};

float getElevation(vec2 coord, float bias) {
    // Convert encoded elevation value to meters
    vec4 data = texture(u_image, coord) * 255.0;
    data.a = -1.0;
    return dot(data, u_unpack);
}

void main() {
    vec2 epsilon = 1.0 / u_dimension;
    // u_dimension is the DEM buffer stride (interior + 2 * 3-pixel border),
    // so the actual interior tile size is stride - 6.
    float tileSize = u_dimension.x - 6.0;

    // queried pixels (using Sobel operator kernel):
    // +-----------+
    // |   |   |   |
    // | a | b | c |
    // |   |   |   |
    // +-----------+
    // |   |   |   |
    // | d | e | f |
    // |   |   |   |
    // +-----------+
    // |   |   |   |
    // | g | h | i |
    // |   |   |   |
    // +-----------+

    float a = getElevation(v_pos + vec2(-epsilon.x, -epsilon.y), 0.0);
    float b = getElevation(v_pos + vec2(0, -epsilon.y), 0.0);
    float c = getElevation(v_pos + vec2(epsilon.x, -epsilon.y), 0.0);
    float d = getElevation(v_pos + vec2(-epsilon.x, 0), 0.0);
  //float e = getElevation(v_pos, 0.0);
    float f = getElevation(v_pos + vec2(epsilon.x, 0), 0.0);
    float g = getElevation(v_pos + vec2(-epsilon.x, epsilon.y), 0.0);
    float h = getElevation(v_pos + vec2(0, epsilon.y), 0.0);
    float i = getElevation(v_pos + vec2(epsilon.x, epsilon.y), 0.0);

    // Convert the raw pixel-space derivative (slope) into world-space slope.
    // The conversion factor is: tileSize / (8 * meters_per_pixel).
    // meters_per_pixel is calculated as pow(2.0, 28.2562 - u_zoom).
    // The exaggeration factor is applied to scale the effect at lower zooms.
    // See nickidlugash's awesome breakdown for more info
    // https://github.com/mapbox/mapbox-gl-js/pull/5286#discussion_r148419556
    float exaggerationFactor = u_zoom < 2.0 ? 0.4 : u_zoom < 4.5 ? 0.35 : 0.3;
    float exaggeration = u_zoom < 15.0 ? (u_zoom - 15.0) * exaggerationFactor : 0.0;

    vec2 deriv = vec2(
        (c + f + f + i) - (a + d + d + g),
        (g + h + h + i) - (a + b + b + c)
    ) * tileSize / pow(2.0, exaggeration + (28.2562 - u_zoom));

    // Encode the derivative into the color channels (r and g)
    // The derivative is scaled from world-space slope to the range [0, 1] for texture storage.
    // The maximum possible world-space derivative is assumed to be 4 (hence division by 8.0).
    fragColor = clamp(vec4(
        deriv.x / 8.0 + 0.5,
        deriv.y / 8.0 + 0.5,
        1.0,
        1.0), 0.0, 1.0);
#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
