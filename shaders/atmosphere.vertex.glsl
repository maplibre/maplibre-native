layout(location = 0) in vec2 a_pos;

layout(std140) uniform AtmospherePropsUBO {
    mat4 inv_projection;
    vec4 globe;
    vec4 sun_position;
} atmosphere;

out vec3 v_view_direction;

void main() {
    v_view_direction = (atmosphere.inv_projection * vec4(a_pos, 0.0, 1.0)).xyz;
    // WebGL maps NDC z=0 to depth 0.5. This keeps atmospheric scattering in
    // front of the planet while still participating in the read-only depth test.
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
