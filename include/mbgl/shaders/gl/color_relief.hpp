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
    float pad_tile0;
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
    float pad_tile0;
};

layout(std140) uniform ColorReliefEvaluatedPropsUBO {
    float u_opacity;
    float pad_eval0;
    float pad_eval1;
    float pad_eval2;
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

float getElevationStop(int stop) {
    // Elevation stops are plain float values, not terrain-RGB encoded
    float x = (float(stop) + 0.5) / float(u_color_ramp_size);
    return texture(u_elevation_stops, vec2(x, 0.0)).g;
}

vec4 getColorStop(int stop) {
    float x = (float(stop) + 0.5) / float(u_color_ramp_size);
    return texture(u_color_stops, vec2(x, 0.0));
}

void main() {
    // --- TEMPORARY DEBUG CODE START ---

    // 1. Sample the middle of the u_elevation_stops texture (a 1D texture)
    //    We know the data should contain non-zero values here (up to 3000m).
    vec4 raw_data = texture(u_elevation_stops, vec2(0.5, 0.0));
    
    // 2. Normalize the values to the display range [0.0, 1.0].
    //    We'll assume the max possible elevation stop is 3000.0 based on your logs.
    //    This scaling is essential because values over 1.0 (e.g., 1000.0 meters) 
    //    will just render as pure white (saturated).
    float normalize_factor = 1.0 / 3000.0;
    
    // 3. Output the raw R, G, B, A components of the sampled vec4, scaled down.
    //    The screen will now be colored based on which channel contains the data.
    gl_FragColor = raw_data * normalize_factor; 

    // --- TEMPORARY DEBUG CODE END ---

    // ** IMPORTANT: Remove this debug code and restore your original main() 
    //    functionality after the test.
})";
};

} // namespace shaders
} // namespace mbgl
