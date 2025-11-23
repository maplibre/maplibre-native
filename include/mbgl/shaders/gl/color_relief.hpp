// Generated code, do not modify this file!
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::ColorReliefShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "ColorReliefShader";
    static constexpr const char* vertex = R"(layout(std140) uniform ColorReliefDrawableUBO {
    highp mat4 u_matrix;
};

layout(std140) uniform ColorReliefTilePropsUBO {
    highp vec4 u_unpack;
    highp vec2 u_dimension;
    int u_color_ramp_size;
    float pad0;
};

in vec2 a_pos;
out vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);

    highp vec2 epsilon = 1.0 / u_dimension;
    float scale = (u_dimension.x - 2.0) / u_dimension.x;
    v_pos = (a_pos / 8192.0) * scale + epsilon;

    // Handle poles
    if (a_pos.y < -32767.5) v_pos.y = 0.0;
    if (a_pos.y > 32766.5) v_pos.y = 1.0;
}
)";
    static constexpr const char* fragment = R"(layout(std140) uniform ColorReliefDrawableUBO {
    highp mat4 u_matrix;
};

layout(std140) uniform ColorReliefTilePropsUBO {
    highp vec4 u_unpack;
    highp vec2 u_dimension;
    int u_color_ramp_size;
    float pad0;
};

layout(std140) uniform ColorReliefEvaluatedPropsUBO {
    float u_opacity;
    float pad0;
    float pad1;
    float pad2;
};

uniform sampler2D u_image;
uniform sampler2D u_elevation_stops;
uniform sampler2D u_color_stops;

in vec2 v_pos;

float getElevation(vec2 coord) {
    vec4 data = texture(u_image, coord) * 255.0;
    data.a = -1.0;
    return dot(data, u_unpack);
}

float getElevationStop(int stop) {
    float x = (float(stop) + 0.5) / float(u_color_ramp_size);
    vec4 data = texture(u_elevation_stops, vec2(x, 0.0)) * 255.0;
    data.a = -1.0;
    return dot(data, u_unpack);
}

void main() {
    float el = getElevation(v_pos);

    // Binary search for color stops
    int r = (u_color_ramp_size - 1);
    int l = 0;
    float el_l = getElevationStop(l);
    float el_r = getElevationStop(r);

    while(r - l > 1) {
        int m = (r + l) / 2;
        float el_m = getElevationStop(m);
        if(el < el_m) {
            r = m;
            el_r = el_m;
        } else {
            l = m;
            el_l = el_m;
        }
    }

    float x = (float(l) + (el - el_l) / (el_r - el_l) + 0.5) / float(u_color_ramp_size);
    fragColor = u_opacity * texture(u_color_stops, vec2(x, 0.0));

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
