// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "FillExtrusionPatternShader";

    static constexpr const char* vertexData = R"(layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_normal_ed;

out vec2 v_pos_a;
out vec2 v_pos_b;
out vec4 v_lighting;

layout (std140) uniform FillExtrusionDrawableTilePropsUBO {
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
};
layout (std140) uniform FillExtrusionInterpolateUBO {
    highp float u_base_t;
    highp float u_height_t;
    highp float u_color_t;
    highp float u_pattern_from_t;
    highp float u_pattern_to_t;
    highp float u_pad_interp1, u_pad_interp2, u_pad_interp3;
};
layout (std140) uniform FillExtrusionDrawableUBO {
    highp mat4 u_matrix;
    highp vec4 u_scale;
    highp vec2 u_texsize;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp float u_height_factor;
    highp float u_pad_drawable;
};
layout (std140) uniform FillExtrusionDrawablePropsUBO {
    highp vec4 u_color;
    highp vec3 u_lightcolor;
    highp float u_pad1;
    highp vec3 u_lightpos;
    highp float u_base;
    highp float u_height;
    highp float u_lightintensity;
    highp float u_vertical_gradient;
    highp float u_opacity;
    highp float u_fade;
    highp float u_pad_props2, u_pad_props3, u_pad_props4;
};

#ifndef HAS_UNIFORM_u_base
layout (location = 2) in lowp vec2 a_base;
out lowp float base;
#endif
#ifndef HAS_UNIFORM_u_height
layout (location = 3) in lowp vec2 a_height;
out lowp float height;
#endif
#ifndef HAS_UNIFORM_u_pattern_from
layout (location = 4) in mediump vec4 a_pattern_from;
out mediump vec4 pattern_from;
#endif
#ifndef HAS_UNIFORM_u_pattern_to
layout (location = 5) in mediump vec4 a_pattern_to;
out mediump vec4 pattern_to;
#endif

void main() {
    #ifndef HAS_UNIFORM_u_base
base = unpack_mix_vec2(a_base, u_base_t);
#else
lowp float base = u_base;
#endif
    #ifndef HAS_UNIFORM_u_height
height = unpack_mix_vec2(a_height, u_height_t);
#else
lowp float height = u_height;
#endif
    #ifndef HAS_UNIFORM_u_pattern_from
pattern_from = a_pattern_from;
#else
mediump vec4 pattern_from = u_pattern_from;
#endif
    #ifndef HAS_UNIFORM_u_pattern_to
pattern_to = a_pattern_to;
#else
mediump vec4 pattern_to = u_pattern_to;
#endif

    vec2 pattern_tl_a = pattern_from.xy;
    vec2 pattern_br_a = pattern_from.zw;
    vec2 pattern_tl_b = pattern_to.xy;
    vec2 pattern_br_b = pattern_to.zw;

    float pixelRatio = u_scale.x;
    float tileRatio = u_scale.y;
    float fromScale = u_scale.z;
    float toScale = u_scale.w;

    vec3 normal = a_normal_ed.xyz;
    float edgedistance = a_normal_ed.w;

    vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    base = max(0.0, base);
    height = max(0.0, height);

    float t = mod(normal.x, 2.0);
    float z = t > 0.0 ? height : base;

    gl_Position = u_matrix * vec4(a_pos, z, 1);

    vec2 pos = normal.x == 1.0 && normal.y == 0.0 && normal.z == 16384.0
        ? a_pos // extrusion top
        : vec2(edgedistance, z * u_height_factor); // extrusion side

    v_pos_a = get_pattern_pos(u_pixel_coord_upper, u_pixel_coord_lower, fromScale * display_size_a, tileRatio, pos);
    v_pos_b = get_pattern_pos(u_pixel_coord_upper, u_pixel_coord_lower, toScale * display_size_b, tileRatio, pos);

    v_lighting = vec4(0.0, 0.0, 0.0, 1.0);
    float directional = clamp(dot(normal / 16383.0, u_lightpos), 0.0, 1.0);
    directional = mix((1.0 - u_lightintensity), max((0.5 + u_lightintensity), 1.0), directional);

    if (normal.y != 0.0) {
        // This avoids another branching statement, but multiplies by a constant of 0.84 if no vertical gradient,
        // and otherwise calculates the gradient based on base + height
        directional *= (
            (1.0 - u_vertical_gradient) +
            (u_vertical_gradient * clamp((t + base) * pow(height / 150.0, 0.5), mix(0.7, 0.98, 1.0 - u_lightintensity), 1.0)));
    }

    v_lighting.rgb += clamp(directional * u_lightcolor, mix(vec3(0.0), vec3(0.3), 1.0 - u_lightcolor), vec3(1.0));
    v_lighting *= u_opacity;
}
)";
    static constexpr const char* fragmentData = R"(in vec2 v_pos_a;
in vec2 v_pos_b;
in vec4 v_lighting;

layout (std140) uniform FillExtrusionDrawableTilePropsUBO {
    highp vec4 u_pattern_from;
    highp vec4 u_pattern_to;
};
layout (std140) uniform FillExtrusionInterpolateUBO {
    highp float u_base_t;
    highp float u_height_t;
    highp float u_color_t;
    highp float u_pattern_from_t;
    highp float u_pattern_to_t;
    highp float u_pad_interp1, u_pad_interp2, u_pad_interp3;
};
layout (std140) uniform FillExtrusionDrawableUBO {
    highp mat4 u_matrix;
    highp vec4 u_scale;
    highp vec2 u_texsize;
    highp vec2 u_pixel_coord_upper;
    highp vec2 u_pixel_coord_lower;
    highp float u_height_factor;
    highp float u_pad_drawable;
};
layout (std140) uniform FillExtrusionDrawablePropsUBO {
    highp vec4 u_color;
    highp vec3 u_lightcolor;
    highp float u_pad1;
    highp vec3 u_lightpos;
    highp float u_base;
    highp float u_height;
    highp float u_lightintensity;
    highp float u_vertical_gradient;
    highp float u_opacity;
    highp float u_fade;
    highp float u_pad_props2, u_pad_props3, u_pad_props4;
};

uniform sampler2D u_image;

#ifndef HAS_UNIFORM_u_base
in lowp float base;
#endif
#ifndef HAS_UNIFORM_u_height
in lowp float height;
#endif
#ifndef HAS_UNIFORM_u_pattern_from
in mediump vec4 pattern_from;
#endif
#ifndef HAS_UNIFORM_u_pattern_to
in mediump vec4 pattern_to;
#endif

void main() {
    #ifdef HAS_UNIFORM_u_base
lowp float base = u_base;
#endif
    #ifdef HAS_UNIFORM_u_height
lowp float height = u_height;
#endif
    #ifdef HAS_UNIFORM_u_pattern_from
mediump vec4 pattern_from = u_pattern_from;
#endif
    #ifdef HAS_UNIFORM_u_pattern_to
mediump vec4 pattern_to = u_pattern_to;
#endif

    vec2 pattern_tl_a = pattern_from.xy;
    vec2 pattern_br_a = pattern_from.zw;
    vec2 pattern_tl_b = pattern_to.xy;
    vec2 pattern_br_b = pattern_to.zw;

    vec2 imagecoord = mod(v_pos_a, 1.0);
    vec2 pos = mix(pattern_tl_a / u_texsize, pattern_br_a / u_texsize, imagecoord);
    vec4 color1 = texture(u_image, pos);

    vec2 imagecoord_b = mod(v_pos_b, 1.0);
    vec2 pos2 = mix(pattern_tl_b / u_texsize, pattern_br_b / u_texsize, imagecoord_b);
    vec4 color2 = texture(u_image, pos2);

    vec4 mixedColor = mix(color1, color2, u_fade);

    fragColor = mixedColor * v_lighting;

#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
    static std::string vertex() {
        using Ty = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL>;
        return Ty::vertexData;
    }
    static std::string fragment() {
        using Ty = ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::OpenGL>;
        return Ty::fragmentData;
    }
};

} // namespace shaders
} // namespace mbgl

// NOLINTEND
