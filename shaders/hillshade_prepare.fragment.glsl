#ifdef GL_ES
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

float getElevation(ivec2 texel) {
    // Convert encoded elevation value to meters
    vec4 data = texelFetch(u_image, texel, 0) * 255.0;
    data.a = -1.0;
    return dot(data, u_unpack);
}

void main() {
    // v_pos sits on the centre of the DEM texel this fragment derives from - the vertex
    // shader maps the ring-inclusive target onto texels 1..dim+2 - so floor picks it
    // exactly. Taken from v_pos rather than the fragment position builtin, whose y origin
    // differs between GL and Metal/Vulkan/WebGPU.
    ivec2 texel = ivec2(floor(v_pos * u_dimension));
    float tileSize = u_dimension.x - 4.0;

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

    float a = getElevation(texel + ivec2(-1, -1));
    float b = getElevation(texel + ivec2(0, -1));
    float c = getElevation(texel + ivec2(1, -1));
    float d = getElevation(texel + ivec2(-1, 0));
  //float e = getElevation(texel);
    float f = getElevation(texel + ivec2(1, 0));
    float g = getElevation(texel + ivec2(-1, 1));
    float h = getElevation(texel + ivec2(0, 1));
    float i = getElevation(texel + ivec2(1, 1));

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
