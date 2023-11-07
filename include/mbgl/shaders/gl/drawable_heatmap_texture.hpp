// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>


namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "HeatmapTextureShader";


    static constexpr const char* vertexData = R"(layout (location = 0) in vec2 a_pos;
out vec2 v_pos;

layout (std140) uniform HeatmapTextureDrawableUBO {
    highp mat4 u_matrix;
    highp vec2 u_world;
    highp float u_opacity;
    lowp float pad0_;
};

void main() {
    gl_Position = u_matrix * vec4(a_pos * u_world, 0, 1);

    v_pos.x = a_pos.x;
    v_pos.y = 1.0 - a_pos.y;
}
)";
    static constexpr const char* fragmentData = R"(in vec2 v_pos;
uniform sampler2D u_image;
uniform sampler2D u_color_ramp;

layout (std140) uniform HeatmapTextureDrawableUBO {
    highp mat4 u_matrix;
    highp vec2 u_world;
    highp float u_opacity;
    lowp float pad0_;
};

void main() {
    float t = texture(u_image, v_pos).r;
    vec4 color = texture(u_color_ramp, vec2(t, 0.5));
    fragColor = color * u_opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(0.0);
#endif
}
)";
    static std::string vertex() {
        using Ty = ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL>;
        return Ty::vertexData;
    }
    static std::string fragment() {
        using Ty = ShaderSource<BuiltIn::HeatmapTextureShader, gfx::Backend::Type::OpenGL>;
        return Ty::fragmentData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
