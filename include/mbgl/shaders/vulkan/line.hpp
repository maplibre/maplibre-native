#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::LineShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LineShader";

    static const std::array<UniformBlockInfo, 4> uniforms;
    static const std::array<AttributeInfo, 8> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

    layout(location = 0) in vec2 in_pos_normal;
    layout(location = 1) in vec4 in_data;

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 2) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
    layout(location = 3) in vec2 in_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 4) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_gapwidth)
    layout(location = 5) in vec2 in_gapwidth;
#endif

#if !defined(HAS_UNIFORM_u_offset)
    layout(location = 6) in vec2 in_offset;
#endif

#if !defined(HAS_UNIFORM_u_width)
    layout(location = 7) in vec2 in_width;
#endif

    layout(set = 0, binding = 1) uniform LineDrawableUBO {
        mat4 matrix;
        mediump float ratio;
        float pad1, pad2, pad3;
    } drawable;

    layout(set = 0, binding = 2) uniform LineInterpolationUBO {
        float color_t;
        float blur_t;
        float opacity_t;
        float gapwidth_t;
        float offset_t;
        float width_t;
        float pad1, pad2;
    } interp;

    layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
        vec4 color;
        float blur;
        float opacity;
        float gapwidth;
        float offset;
        float width;
        float floorwidth;
        uint expressionMask;
        float pad1;
    } props;

    layout(location = 0) out lowp vec2 frag_normal;
    layout(location = 1) out lowp vec2 frag_width2;
    layout(location = 2) out lowp float frag_gamma_scale;

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 4) out lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
    layout(location = 5) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 6) out lowp float frag_opacity;
#endif

    void main() {

#ifndef HAS_UNIFORM_u_color
        frag_color = unpack_mix_color(in_color, interp.color_t);
#endif

#ifndef HAS_UNIFORM_u_blur
        frag_blur = unpack_mix_vec2(in_blur, interp.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
        frag_opacity = unpack_mix_vec2(in_opacity, interp.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_gapwidth
        mediump float gapwidth = unpack_mix_vec2(in_gapwidth, interp.gapwidth_t);
#else
        mediump float gapwidth = props.gapwidth;
#endif

#ifndef HAS_UNIFORM_u_offset
        lowp float offset = unpack_mix_vec2(in_offset, interp.offset_t);
#else
        lowp float offset = props.offset;
#endif
        
#ifndef HAS_UNIFORM_u_width
        mediump float width = unpack_mix_vec2(in_width, interp.width_t);
#else
        mediump float width = props.width;
#endif

        // the distance over which the line edge fades out.
        // Retina devices need a smaller distance to avoid aliasing.
        float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

        vec2 a_extrude = in_data.xy - 128.0;
        float a_direction = mod(in_data.z, 4.0) - 1.0;

        vec2 pos = floor(in_pos_normal * 0.5);

        // x is 1 if it's a round cap, 0 otherwise
        // y is 1 if the normal points up, and -1 if it points down
        // We store these in the least significant bit of in_pos_normal
        mediump vec2 normal = in_pos_normal - 2.0 * pos;
        normal.y = normal.y * 2.0 - 1.0;
        frag_normal = normal;

        // these transformations used to be applied in the JS and native code bases.
        // moved them into the shader for clarity and simplicity.
        gapwidth = gapwidth / 2.0;
        float halfwidth = width / 2.0;
        offset = -1.0 * offset;

        float inset = gapwidth + (gapwidth > 0.0 ? ANTIALIASING : 0.0);
        float outset = gapwidth + halfwidth * (gapwidth > 0.0 ? 2.0 : 1.0) + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

        // Scale the extrusion vector down to a normal and then up by the line width
        // of this vertex.
        mediump vec2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

        // Calculate the offset when drawing a line that is to the side of the actual line.
        // We do this by creating a vector that points towards the extrude, but rotate
        // it when we're drawing round end points (a_direction = -1 or 1) since their
        // extrude vector points in another direction.
        mediump float u = 0.5 * a_direction;
        mediump float t = 1.0 - abs(u);
        mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * normal.y * mat2(t, -u, u, t);

        vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
        gl_Position = drawable.matrix * vec4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;
        gl_Position.y *= -1.0;

        // calculate how much the perspective view squishes or stretches the extrude
        float extrude_length_without_perspective = length(dist);
        float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * global.units_to_pixels);
        frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

        frag_width2 = vec2(outset, inset);
    }
)";

    static constexpr auto fragment = R"(

    layout(location = 0) in lowp vec2 frag_normal;
    layout(location = 1) in lowp vec2 frag_width2;
    layout(location = 2) in lowp float frag_gamma_scale;

#if !defined(HAS_UNIFORM_u_color)
    layout(location = 4) in lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
    layout(location = 5) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    layout(location = 6) in lowp float frag_opacity;
#endif

    layout(location = 0) out vec4 outColor;

    layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
        vec4 color;
        float blur;
        float opacity;
        float gapwidth;
        float offset;
        float width;
        float floorwidth;
        uint expressionMask;
        float pad1;
    } props;

    void main() {

#ifdef OVERDRAW_INSPECTOR
        outColor = vec4(1.0);
        return;
#endif

#ifdef HAS_UNIFORM_u_color
        highp vec4 color = props.color;
#else
        highp vec4 color = frag_color;
#endif

#ifdef HAS_UNIFORM_u_blur
        lowp float blur = props.blur;
#else
        lowp float blur = frag_blur;
#endif
        
#ifdef HAS_UNIFORM_u_opacity
        lowp float opacity = props.opacity;
#else
        lowp float opacity = frag_opacity;
#endif

        // Calculate the distance of the pixel from the line in pixels.
        float dist = length(frag_normal) * frag_width2.s;

        // Calculate the antialiasing fade factor. This is either when fading in
        // the line in case of an offset line (frag_width2.t) or when fading out
        // (frag_width2.s)
        float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;
        float alpha = clamp(min(dist - (frag_width2.t - blur2), frag_width2.s - dist) / blur2, 0.0, 1.0);

        outColor = color * (alpha * opacity);
    }
)";
};

} // namespace shaders
} // namespace mbgl
