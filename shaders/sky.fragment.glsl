#ifdef GL_ES
precision highp float;
#endif

in vec2 v_pos;
in vec3 v_view_direction;

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

void main() {
    vec2 pixel = (v_pos * 0.5 + 0.5) * sky.viewport_size;
    float blend = dot(pixel - sky.horizon, sky.horizon_normal);
    float opacity = 1.0;
    if (sky.sky_blend > 0.0) {
        vec3 ray = normalize(v_view_direction);
        vec3 globe_position = sky.globe.xyz;
        float globe_radius = sky.globe.w;
        float globe_distance = length(globe_position);
        float angle_to_globe_center = acos(clamp(dot(ray, globe_position) / globe_distance, -1.0, 1.0));
        float horizon_angle = asin(min(globe_radius / globe_distance, 1.0));
        blend = mix(blend, ((angle_to_globe_center - horizon_angle) * sky.camera_to_center_distance + 1.0) * sky.pixel_ratio, sky.sky_blend);
        float closest_approach = max(dot(ray, globe_position), 0.0);
        float lowest_altitude = sqrt(max(globe_distance * globe_distance - closest_approach * closest_approach, 0.0)) - globe_radius;
        float atmosphere_height = sky.atmosphere_height;
        float inside_atmosphere = 1.0 - smoothstep(atmosphere_height, 10.0 * atmosphere_height, globe_distance - globe_radius);
        float through_atmosphere = 1.0 - smoothstep(0.0, atmosphere_height, lowest_altitude);
        opacity = inside_atmosphere * through_atmosphere * (1.0 - sky.atmosphere_blend);
    }

    vec4 color = vec4(0.0);
    if (blend > 0.0) {
        if (blend < sky.sky_horizon_blend) {
            float t = 1.0 - blend / sky.sky_horizon_blend;
            color = mix(sky.sky_color, sky.horizon_color, t * t);
        } else {
            color = sky.sky_color;
        }
    }

    fragColor = color * mix(1.0, opacity, sky.sky_blend);
}
