// Generated code, do not modify this file!
#pragma once
#include <mln/shaders/shader_source.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillExtrusionShadowMaskShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "FillExtrusionShadowMaskShader";
    static constexpr const char* vertex = R"(layout (location = 0) in vec2 a_pos;
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

#ifndef HAS_UNIFORM_u_base
layout (location = 2) in highp vec2 a_base;
#endif
#ifndef HAS_UNIFORM_u_height
layout (location = 3) in highp vec2 a_height;
#endif

void main() {
    #ifndef HAS_UNIFORM_u_base
highp float base = unpack_mix_vec2(a_base, u_base_t);
#else
highp float base = u_base;
#endif
    #ifndef HAS_UNIFORM_u_height
highp float height = unpack_mix_vec2(a_height, u_height_t);
#else
highp float height = u_height;
#endif

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
)";
    static constexpr const char* fragment = R"(layout (std140) uniform FillExtrusionShadowPropsUBO {
    highp vec4 u_color;
    highp float u_opacity;
    highp float u_base;
    highp float u_height;
};

void main() {
#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
    return;
#endif

    // u_color is already premultiplied, so scaling the whole vector keeps the alpha blend correct.
    fragColor = u_color * u_opacity;
}
)";
};

} // namespace shaders
} // namespace mln
