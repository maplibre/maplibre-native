layout(location = 0) in vec2 a_pos;

layout(std140) uniform SkyPropsUBO {
    mat4 inv_projection;
    vec4 sky_color;
    vec4 horizon_color;
    vec4 globe;
    vec2 horizon;
    vec2 horizon_normal;
    vec2 viewport_size;
    float sky_horizon_blend;
    float sky_blend;
    float camera_to_center_distance;
    float pixel_ratio;
    float atmosphere_blend;
    float atmosphere_height;
} sky;

out vec2 v_pos;
out vec3 v_view_direction;

void main() {
    v_pos = a_pos;
    v_view_direction = (sky.inv_projection * vec4(a_pos, 0.0, 1.0)).xyz;
    gl_Position = vec4(a_pos, 1.0, 1.0);
}
