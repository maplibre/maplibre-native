// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "FillShader";

    static constexpr const char* vertexData = R"(layout (std140) uniform FillDrawableUBO {
    highp mat4 u_matrix;
};
layout (std140) uniform FillEvaluatedPropsUBO {
    highp vec4 u_color;
    highp float u_opacity;
    highp float padding_props1;
    highp float padding_props2;
    highp float padding_props3;
};
layout (std140) uniform FillInterpolateUBO {
    highp float u_color_t;
    highp float u_opacity_t;
    highp float pad_interp_1, pad_interp_2;
};

layout (location = 0) in vec2 a_pos;

#ifndef HAS_UNIFORM_u_color
layout (location = 1) in highp vec4 a_color;
out highp vec4 color;
#endif
#ifndef HAS_UNIFORM_u_opacity
layout (location = 2) in lowp vec2 a_opacity;
out lowp float opacity;
#endif

void main() {
    #ifndef HAS_UNIFORM_u_color
color = unpack_mix_color(a_color, u_color_t);
#else
highp vec4 color = u_color;
#endif
    #ifndef HAS_UNIFORM_u_opacity
opacity = unpack_mix_vec2(a_opacity, u_opacity_t);
#else
lowp float opacity = u_opacity;
#endif

    gl_Position = u_matrix * vec4(a_pos, 0, 1);
}
)";
    static constexpr const char* fragmentData = R"(layout (std140) uniform FillEvaluatedPropsUBO {
    highp vec4 u_color;
    highp float u_opacity;
    highp float padding_props1;
    highp float padding_props2;
    highp float padding_props3;
};
layout (std140) uniform FillInterpolateUBO {
    highp float u_color_t;
    highp float u_opacity_t;
    highp float pad_interp_1, pad_interp_2;
};

#ifndef HAS_UNIFORM_u_color
in highp vec4 color;
#endif
#ifndef HAS_UNIFORM_u_opacity
in lowp float opacity;
#endif

void main() {
    #ifdef HAS_UNIFORM_u_color
highp vec4 color = u_color;
#endif
    #ifdef HAS_UNIFORM_u_opacity
lowp float opacity = u_opacity;
#endif

    fragColor = color * opacity;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
    static std::string vertex() {
        using Ty = ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::OpenGL>;
        return Ty::vertexData;
    }
    static std::string fragment() {
        using Ty = ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::OpenGL>;
        return Ty::fragmentData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
