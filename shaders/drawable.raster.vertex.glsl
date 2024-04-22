layout (std140) uniform RasterDrawableUBO {
    highp mat4 u_matrix;
};
layout (std140) uniform RasterEvaluatedPropsUBO {
    highp vec3 u_spin_weights;
    highp vec2 u_tl_parent;
    highp float u_scale_parent;
    highp float u_buffer_scale;
    highp float u_fade_t;
    highp float u_opacity;
    highp float u_brightness_low;
    highp float u_brightness_high;
    highp float u_saturation_factor;
    highp float u_contrast_factor;
    highp float props_pad1;
    highp float props_pad2;
};

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;

out vec2 v_pos0;
out vec2 v_pos1;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    // We are using Int16 for texture position coordinates to give us enough precision for
    // fractional coordinates. We use 8192 to scale the texture coordinates in the buffer
    // as an arbitrarily high number to preserve adequate precision when rendering.
    // This is also the same value as the EXTENT we are using for our tile buffer pos coordinates,
    // so math for modifying either is consistent.
    v_pos0 = (((a_texture_pos / 8192.0) - 0.5) / u_buffer_scale ) + 0.5;
    v_pos1 = (v_pos0 * u_scale_parent) + u_tl_parent;
}
