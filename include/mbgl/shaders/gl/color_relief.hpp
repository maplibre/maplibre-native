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
    highp int u_color_ramp_size;
    highp float pad_tile0;
};

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;
out vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);

    highp vec2 epsilon = 1.0 / u_dimension;
    float scale = (u_dimension.x - 2.0) / u_dimension.x;
    v_pos = (a_texture_pos / 8192.0) * scale + epsilon;

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
    highp int u_color_ramp_size;
    highp float pad_tile0;
};

layout(std140) uniform ColorReliefEvaluatedPropsUBO {
    highp float u_opacity;
    highp float pad_eval0;
    highp float pad_eval1;
    highp float pad_eval2;
};

uniform sampler2D u_image;
uniform sampler2D u_elevation_stops;
uniform sampler2D u_color_stops;

in vec2 v_pos;

float getElevation(vec2 coord) {
    // Convert encoded elevation value to meters
    vec4 data = texture(u_image, coord) * 255.0;
    data.a = -1.0;
    return dot(data, u_unpack);
}

float getElevationStop(int stop, int color_ramp_size) {
    // Elevation stops are packed as IEEE 754 float bytes into RGBA8 (big-endian: R=MSB, A=LSB).
    // RGBA8 is universally supported; RGBA32F sampled images are not mandatory in Vulkan.
    float x = (float(stop) + 0.5) / float(color_ramp_size);
    // round() is critical: texture() returns normalized floats (byte/255.0), and the
    // multiply-back can produce e.g. 64.999 instead of 65.0. Without round(), uint()
    // truncation would corrupt the IEEE 754 bit pattern and produce wrong elevation values.
    vec4 enc = round(texture(u_elevation_stops, vec2(x, 0.5)) * 255.0);
    uint bits = (uint(enc.r) << 24u) | (uint(enc.g) << 16u) | (uint(enc.b) << 8u) | uint(enc.a);
    return uintBitsToFloat(bits);
}

vec4 getColorStop(int stop) {
    float x = (float(stop) + 0.5) / float(u_color_ramp_size);
    return texture(u_color_stops, vec2(x, 0.5));
}

void main() {
    float el = getElevation(v_pos);

    // Binary search for color stops
    int r = (u_color_ramp_size - 1);
    int l = 0;

    while (r - l > 1) {
        int m = (r + l) / 2;
        float el_m = getElevationStop(m, u_color_ramp_size);
        if (el < el_m) {
            r = m;
        } else {
            l = m;
        }
    }

    // Get elevation values for interpolation
    float el_l = getElevationStop(l, u_color_ramp_size);
    float el_r = getElevationStop(r, u_color_ramp_size);

    // Get colors for interpolation
    vec4 color_l = getColorStop(l);
    vec4 color_r = getColorStop(r);

    // Interpolate between the two colors
    // Guard against division by zero when el_r == el_l
    float denom = el_r - el_l;
    float t = (abs(denom) < 0.0001) ? 0.0 : clamp((el - el_l) / denom, 0.0, 1.0);
    fragColor = u_opacity * mix(color_l, color_r, t);

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
