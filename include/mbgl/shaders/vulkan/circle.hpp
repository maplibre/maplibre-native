#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "CircleShader";

    static const std::array<UniformBlockInfo, 4> uniforms;
    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 1) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_radius)
layout(location = 2) in vec2 in_radius;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 3) in vec2 in_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 4) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_stroke_color)
layout(location = 5) in vec4 in_stroke_color;
#endif

#if !defined(HAS_UNIFORM_u_stroke_width)
layout(location = 6) in vec2 in_stroke_width;
#endif

#if !defined(HAS_UNIFORM_u_stroke_opacity)
layout(location = 7) in vec2 in_stroke_opacity;
#endif

layout(set = 0, binding = 1) uniform CircleDrawableUBO {
    mat4 matrix;
    vec2 extrude_scale;
    vec2 padding;
} drawable;

layout(set = 0, binding = 2) uniform CircleEvaluatedPropsUBO {
    vec4 color;
    vec4 stroke_color;
    float radius;
    float blur;
    float opacity;
    float stroke_width;
    float stroke_opacity;
    bool scale_with_map;
    bool pitch_with_map;
    float padding;
} props;

layout(set = 0, binding = 3) uniform CircleInterpolateUBO {
    float color_t;
    float radius_t;
    float blur_t;
    float opacity_t;
    float stroke_color_t;
    float stroke_width_t;
    float stroke_opacity_t;
    float pad1_;
} interp;

layout(location = 0) out vec2 frag_extrude;
layout(location = 1) out float frag_antialiasblur;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 2) out vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_radius)
layout(location = 3) out mediump float frag_radius;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) out lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_stroke_color)
layout(location = 6) out vec4 frag_stroke_color;
#endif

#if !defined(HAS_UNIFORM_u_stroke_width)
layout(location = 7) out mediump float frag_stroke_width;
#endif

#if !defined(HAS_UNIFORM_u_stroke_opacity)
layout(location = 8) out lowp float frag_stroke_opacity;
#endif

void main() {

#if defined(HAS_UNIFORM_u_radius)
    const float radius = props.radius;
#else
    const float radius = unpack_mix_float(in_radius, interp.radius_t);
#endif

#if defined(HAS_UNIFORM_u_stroke_width)
    const float stroke_width = props.stroke_width;
#else
    const float stroke_width = unpack_mix_float(in_stroke_width, interp.stroke_width_t);
#endif

    // unencode the extrusion vector that we snuck into the a_pos vector
    const vec2 extrude = mod(in_position, 2.0) * 2.0 - 1.0;
    const vec2 scaled_extrude = extrude * drawable.extrude_scale;

    // multiply a_pos by 0.5, since we had it * 2 in order to sneak in extrusion data
    const vec2 circle_center = floor(in_position * 0.5);

    if (props.pitch_with_map) {
        vec2 corner_position = circle_center;
        if (props.scale_with_map) {
            corner_position += scaled_extrude * (radius + stroke_width);
        } else {
            // Pitching the circle with the map effectively scales it with the map
            // To counteract the effect for pitch-scale: viewport, we rescale the
            // whole circle based on the pitch scaling effect at its central point
            const vec4 projected_center = drawable.matrix * vec4(circle_center, 0, 1);
            corner_position += scaled_extrude * (radius + stroke_width) *
                               (projected_center.w / global.camera_to_center_distance);
        }

        gl_Position = drawable.matrix * vec4(corner_position, 0, 1);
    } else {
        gl_Position = drawable.matrix * vec4(circle_center, 0, 1);

        const float factor = props.scale_with_map ? global.camera_to_center_distance : gl_Position.w;
        gl_Position.xy += scaled_extrude * (radius + stroke_width) * factor;
    }

    gl_Position.y *= -1.0;

    // This is a minimum blur distance that serves as a faux-antialiasing for
    // the circle. since blur is a ratio of the circle's size and the intent is
    // to keep the blur at roughly 1px, the two are inversely related.
    frag_antialiasblur = 1.0 / DEVICE_PIXEL_RATIO / (radius + stroke_width);

    frag_extrude = extrude;

#if !defined(HAS_UNIFORM_u_color)
    frag_color = unpack_mix_color(in_color, interp.color_t);
#endif

#if !defined(HAS_UNIFORM_u_radius)
    frag_radius = radius;
#endif

#if !defined(HAS_UNIFORM_u_blur)
    frag_blur = unpack_mix_float(in_blur, interp.blur_t);
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif

#if !defined(HAS_UNIFORM_u_stroke_color)
    frag_stroke_color = unpack_mix_color(in_stroke_color, interp.stroke_color_t);
#endif

#if !defined(HAS_UNIFORM_u_stroke_width)
    frag_stroke_width = stroke_width;
#endif

#if !defined(HAS_UNIFORM_u_stroke_opacity)
    frag_stroke_opacity = unpack_mix_float(in_stroke_opacity, interp.stroke_opacity_t);
#endif
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_extrude;
layout(location = 1) in float frag_antialiasblur;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 2) in vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_radius)
layout(location = 3) in mediump float frag_radius;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) in lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_stroke_color)
layout(location = 6) in vec4 frag_stroke_color;
#endif

#if !defined(HAS_UNIFORM_u_stroke_width)
layout(location = 7) in mediump float frag_stroke_width;
#endif

#if !defined(HAS_UNIFORM_u_stroke_opacity)
layout(location = 8) in lowp float frag_stroke_opacity;
#endif

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 2) uniform CircleEvaluatedPropsUBO {
    vec4 color;
    vec4 stroke_color;
    float radius;
    float blur;
    float opacity;
    float stroke_width;
    float stroke_opacity;
    bool scale_with_map;
    bool pitch_with_map;
    float padding;
} props;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_color)
    const vec4 color = props.color;
#else
    const vec4 color = frag_color;
#endif
#if defined(HAS_UNIFORM_u_radius)
    const float radius = props.radius;
#else
    const float radius = frag_radius;
#endif
#if defined(HAS_UNIFORM_u_blur)
    const float blur = props.blur;
#else
    const float blur = frag_blur;
#endif
#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = frag_opacity;
#endif
#if defined(HAS_UNIFORM_u_stroke_color)
    const vec4 stroke_color = props.stroke_color;
#else
    const vec4 stroke_color = frag_stroke_color;
#endif
#if defined(HAS_UNIFORM_u_stroke_width)
    const float stroke_width = props.stroke_width;
#else
    const float stroke_width = frag_stroke_width;
#endif
#if defined(HAS_UNIFORM_u_stroke_opacity)
    const float stroke_opacity = props.stroke_opacity;
#else
    const float stroke_opacity = frag_stroke_opacity;
#endif

    const float extrude_length = length(frag_extrude);
    const float antialiased_blur = -max(blur, frag_antialiasblur);
    const float opacity_t = smoothstep(0.0, antialiased_blur, extrude_length - 1.0);
    const float color_t = (stroke_width < 0.01) ? 0.0 :
        smoothstep(antialiased_blur, 0.0, extrude_length - radius / (radius + stroke_width));

    out_color = opacity_t * mix(color * opacity, stroke_color * stroke_opacity, color_t);
}
)";
};

} // namespace shaders
} // namespace mbgl
