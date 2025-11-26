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

float getElevation(vec2 coord, float bias) {
    // Convert encoded elevation value to meters
    vec4 data = texture(u_image, coord) * 255.0;
    data.a = -1.0;
    return dot(data, u_unpack);
}

void main() {
    vec2 epsilon = 1.0 / u_dimension;
    float tileSize = u_dimension.x - 2.0;

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
    float exaggeration = u_zoom < 2.0 ? 0.4 : u_zoom < 4.5 ? 0.35 : 0.3;

    vec2 deriv = vec2(
        (c + f + f + i) - (a + d + d + g),
        (g + h + h + i) - (a + b + b + c)
    ) * tileSize / pow(2.0, (u_zoom - u_maxzoom) * exaggeration + 28.2562 - u_zoom);
    
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
