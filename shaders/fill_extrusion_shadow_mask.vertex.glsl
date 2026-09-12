layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_decimals_ed;

layout (std140) uniform FillExtrusionShadowDrawableUBO {
    highp mat4 u_matrix;
    highp vec2 u_offset_per_meter;
    highp float u_base_t;
    highp float u_height_t;
};

layout (std140) uniform FillExtrusionShadowPropsUBO {
    highp vec4 u_color;
    highp float u_opacity;
    highp float u_base;
    highp float u_height;
};

#pragma mapbox: define highp float base
#pragma mapbox: define highp float height

void main() {
    #pragma mapbox: initialize highp float base
    #pragma mapbox: initialize highp float height

    base = max(0.0, base);
    height = max(0.0, height);

    float t = mod(a_decimals_ed.x, 2.0);
    float z = t > 0.0 ? height : base;
    vec2 decimals = unpack_float(floor(a_decimals_ed.x / 2.0)) / 128.0;
    vec2 p = a_pos + decimals;

    // Shear onto the ground plane using this vertex's own z: base for the foot of a wall, height
    // for its top and for roof triangles.
    gl_Position = u_matrix * vec4(p + u_offset_per_meter * z, 0.0, 1.0);
}
