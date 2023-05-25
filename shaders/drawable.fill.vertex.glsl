layout (location = 0) in vec2 a_pos;

layout (std140) uniform FillDrawableUBO {
    mat4 u_matrix;
    vec2 u_world;
    vec2 pad;
};

#pragma mapbox: define highp vec4 color
#pragma mapbox: define lowp float opacity

void main() {
    #pragma mapbox: initialize highp vec4 color
    #pragma mapbox: initialize lowp float opacity

    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
