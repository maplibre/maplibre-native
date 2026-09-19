layout (std140) uniform LocationIndicatorDrawableUBO {
    mat4 u_matrix;
    vec4 u_color;
    vec4 u_sector;
};

in vec2 v_local;

void main() {
    float opacity = 1.0;
    if (u_sector.y > 0.0) {
        float radius = length(v_local);
        float angle = atan(max(abs(v_local.x), 0.000001), -v_local.y);
        float feather = length(fwidth(v_local)) / max(radius, 0.0001);
        float angular = u_sector.x >= 3.14159265 ? 1.0 :
            1.0 - smoothstep(u_sector.x - feather, u_sector.x + feather, angle);
        opacity = angular * (1.0 - smoothstep(0.0, 1.0, radius));
    }
    fragColor = u_color * opacity;
}
