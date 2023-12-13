layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex;

layout (std140) uniform SymbolDrawableUBO {
    highp mat4 u_matrix;
};

// layout (std140) uniform SymbolParametersUBO {
//     highp vec2 u_extrude_scale;
// };


out vec2 v_tex;

void main() {
    highp vec2 u_extrude_scale = vec2(160, -64);

    // unencode the extrusion vector
    vec2 extrude = vec2(mod(a_pos, 2.0) * 2.0 - 1.0);
    
    // get center
    vec2 center = floor(a_pos * 0.5);

    gl_Position = u_matrix * vec4(center, 0, 1);
    gl_Position.xy += extrude * u_extrude_scale;

    v_tex = a_tex;
}
