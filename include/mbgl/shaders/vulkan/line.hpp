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

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

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

layout(set = 0, binding = 4) uniform LineEvaluatedPropsUBO {
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
layout(location = 3) out lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) out lowp float frag_opacity;
#endif

void main() {

#ifndef HAS_UNIFORM_u_color
    frag_color = unpack_mix_color(in_color, interp.color_t);
#endif

#ifndef HAS_UNIFORM_u_blur
    frag_blur = unpack_mix_float(in_blur, interp.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_gapwidth
    const mediump float gapwidth = unpack_mix_float(in_gapwidth, interp.gapwidth_t) / 2.0;
#else
    const mediump float gapwidth = props.gapwidth / 2.0;
#endif

#ifndef HAS_UNIFORM_u_offset
    const lowp float offset = unpack_mix_float(in_offset, interp.offset_t) * -1.0;
#else
    const lowp float offset = props.offset * -1.0;
#endif
        
#ifndef HAS_UNIFORM_u_width
    mediump float width = unpack_mix_float(in_width, interp.width_t);
#else
    mediump float width = props.width;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    vec2 a_extrude = in_data.xy - 128.0;
    float a_direction = mod(in_data.z, 4.0) - 1.0;
    vec2 pos = floor(in_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = width / 2.0;

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
    mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * frag_normal.y * mat2(t, -u, u, t);

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
layout(location = 3) in lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) in lowp float frag_opacity;
#endif

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 4) uniform LineEvaluatedPropsUBO {
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
    out_color = vec4(1.0);
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

    out_color = color * (alpha * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineGradientShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LineGradientShader";

    static const std::array<UniformBlockInfo, 4> uniforms;
    static const std::array<AttributeInfo, 7> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 2) in vec2 in_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 3) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_gapwidth)
layout(location = 4) in vec2 in_gapwidth;
#endif

#if !defined(HAS_UNIFORM_u_offset)
layout(location = 5) in vec2 in_offset;
#endif

#if !defined(HAS_UNIFORM_u_width)
layout(location = 6) in vec2 in_width;
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

layout(set = 0, binding = 4) uniform LineEvaluatedPropsUBO {
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
layout(location = 3) out float frag_lineprogress;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) out lowp float frag_opacity;
#endif

void main() {

#ifndef HAS_UNIFORM_u_blur
    frag_blur = unpack_mix_float(in_blur, interp.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_gapwidth
    mediump float gapwidth = unpack_mix_float(in_gapwidth, interp.gapwidth_t) / 2.0;
#else
    mediump float gapwidth = props.gapwidth / 2.0;
#endif

#ifndef HAS_UNIFORM_u_offset
    const lowp float offset = unpack_mix_float(in_offset, interp.offset_t) * -1.0;
#else
    const lowp float offset = props.offset * -1.0;
#endif
        
#ifndef HAS_UNIFORM_u_width
    mediump float width = unpack_mix_float(in_width, interp.width_t);
#else
    mediump float width = props.width;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    vec2 a_extrude = in_data.xy - 128.0;
    float a_direction = mod(in_data.z, 4.0) - 1.0;
    vec2 pos = floor(in_pos_normal * 0.5);
    frag_lineprogress = (floor(in_data.z / 4.0) + in_data.w * 64.0) * 2.0 / MAX_LINE_DISTANCE;

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = width / 2.0;

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
    mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * frag_normal.y * mat2(t, -u, u, t);

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
layout(location = 3) in float frag_lineprogress;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) in lowp float frag_opacity;
#endif

layout(set = 1, binding = 0) uniform sampler2D image0_sampler;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 4) uniform LineEvaluatedPropsUBO {
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
    out_color = vec4(1.0);
    return;
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

    // For gradient lines, lineprogress is the ratio along the entire line,
    // scaled to [0, 2^15), and the gradient ramp is stored in a texture.
    const vec4 color = texture(image0_sampler, vec2(frag_lineprogress, 0.5));

    out_color = color * (alpha * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LinePatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LinePatternShader";

    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 2) in vec2 in_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 3) in vec2 in_opacity;
#endif

#if !defined(HAS_UNIFORM_u_gapwidth)
layout(location = 4) in vec2 in_gapwidth;
#endif

#if !defined(HAS_UNIFORM_u_offset)
layout(location = 5) in vec2 in_offset;
#endif

#if !defined(HAS_UNIFORM_u_width)
layout(location = 6) in vec2 in_width;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 7) in uvec4 in_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 8) in uvec4 in_pattern_to;
#endif

layout(set = 0, binding = 1) uniform LineDrawableUBO {
    mat4 matrix;
    vec4 scale;
    vec2 texsize;
    float ratio;
    float fade;
} drawable;

layout(set = 0, binding = 2) uniform LineInterpolationUBO {
    float blur_t;
    float opacity_t;
    float offset_t;
    float gapwidth_t;
    float width_t;
    float pattern_from_t;
    float pattern_to_t;
    float pad1;
} interp;

layout(set = 0, binding = 3) uniform LinePatternTilePropertiesUBO {
    vec4 pattern_from;
    vec4 pattern_to;
} tile;

layout(set = 0, binding = 4) uniform LineEvaluatedPropsUBO {
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
layout(location = 3) out float frag_linesofar;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) out lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 6) out mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 7) out mediump vec4 frag_pattern_to;
#endif

void main() {

#ifndef HAS_UNIFORM_u_blur
    frag_blur = unpack_mix_float(in_blur, interp.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_gapwidth
    const mediump float gapwidth = unpack_mix_float(in_gapwidth, interp.gapwidth_t) / 2.0;
#else
    const mediump float gapwidth = props.gapwidth / 2.0;
#endif

#ifndef HAS_UNIFORM_u_offset
    const lowp float offset = unpack_mix_float(in_offset, interp.offset_t) * -1.0;
#else
    const lowp float offset = props.offset * -1.0;
#endif
        
#ifndef HAS_UNIFORM_u_width
    mediump float width = unpack_mix_float(in_width, interp.width_t);
#else
    mediump float width = props.width;
#endif

#ifndef HAS_UNIFORM_u_pattern_from
    frag_pattern_from = in_pattern_from;
#endif

#ifndef HAS_UNIFORM_u_pattern_to
    frag_pattern_to = in_pattern_to;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    vec2 a_extrude = in_data.xy - 128.0;
    float a_direction = mod(in_data.z, 4.0) - 1.0;
    vec2 pos = floor(in_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = width / 2.0;

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
    mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * frag_normal.y * mat2(t, -u, u, t);

    vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
    gl_Position = drawable.matrix * vec4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;
    gl_Position.y *= -1.0;

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * global.units_to_pixels);
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

    frag_width2 = vec2(outset, inset);
    frag_linesofar = (floor(in_data.z / 4.0) + in_data.w * 64.0) * LINE_DISTANCE_SCALE;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in lowp vec2 frag_normal;
layout(location = 1) in lowp vec2 frag_width2;
layout(location = 2) in lowp float frag_gamma_scale;
layout(location = 3) in float frag_linesofar;

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 4) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) in lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 6) in mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 7) in mediump vec4 frag_pattern_to;
#endif

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform LineDrawableUBO {
    mat4 matrix;
    vec4 scale;
    vec2 texsize;
    float ratio;
    float fade;
} drawable;

layout(set = 0, binding = 3) uniform LinePatternTilePropertiesUBO {
    vec4 pattern_from;
    vec4 pattern_to;
} tile;

layout(set = 0, binding = 4) uniform LineEvaluatedPropsUBO {
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

layout(set = 1, binding = 0) uniform sampler2D image0_sampler;

void main() {

#ifdef OVERDRAW_INSPECTOR
    out_color = vec4(1.0);
    return;
#endif

#ifdef HAS_UNIFORM_u_blur
    const lowp float blur = props.blur;
#else
    const lowp float blur = frag_blur;
#endif
        
#ifdef HAS_UNIFORM_u_opacity
    const lowp float opacity = props.opacity;
#else
    const lowp float opacity = frag_opacity;
#endif

#ifdef HAS_UNIFORM_u_pattern_from
    const lowp vec4 pattern_from = tile.pattern_from;
#else
    const lowp vec4 pattern_from = frag_pattern_from;
#endif

#ifdef HAS_UNIFORM_u_pattern_to
    const lowp vec4 pattern_to = tile.pattern_to;
#else
    const lowp vec4 pattern_to = frag_pattern_to;
#endif

    const vec2 pattern_tl_a = pattern_from.xy;
    const vec2 pattern_br_a = pattern_from.zw;
    const vec2 pattern_tl_b = pattern_to.xy;
    const vec2 pattern_br_b = pattern_to.zw;

    const float pixelRatio = drawable.scale.x;
    const float tileZoomRatio = drawable.scale.y;
    const float fromScale = drawable.scale.z;
    const float toScale = drawable.scale.w;

    const vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    const vec2 pattern_size_a = vec2(display_size_a.x * fromScale / tileZoomRatio, display_size_a.y);
    const vec2 pattern_size_b = vec2(display_size_b.x * toScale / tileZoomRatio, display_size_b.y);

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(frag_normal) * frag_width2.x;

    // Calculate the antialiasing fade factor. This is either when fading in
    // the line in case of an offset line (frag_width2.y) or when fading out
    // (frag_width2.x)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;
    const float alpha = clamp(min(dist - (frag_width2.y - blur2), frag_width2.x - dist) / blur2, 0.0, 1.0);

    const float x_a = mod(frag_linesofar / pattern_size_a.x, 1.0);
    const float x_b = mod(frag_linesofar / pattern_size_b.x, 1.0);

    // frag_normal.y is 0 at the midpoint of the line, -1 at the lower edge, 1 at the upper edge
    // we clamp the line width outset to be between 0 and half the pattern height plus padding (2.0)
    // to ensure we don't sample outside the designated symbol on the sprite sheet.
    // 0.5 is added to shift the component to be bounded between 0 and 1 for interpolation of
    // the texture coordinate
    const float y_a = 0.5 + (frag_normal.y * clamp(frag_width2.x, 0.0, (pattern_size_a.y + 2.0) / 2.0) / pattern_size_a.y);
    const float y_b = 0.5 + (frag_normal.y * clamp(frag_width2.x, 0.0, (pattern_size_b.y + 2.0) / 2.0) / pattern_size_b.y);
    const vec2 pos_a = mix(pattern_tl_a / drawable.texsize, pattern_br_a / drawable.texsize, vec2(x_a, y_a));
    const vec2 pos_b = mix(pattern_tl_b / drawable.texsize, pattern_br_b / drawable.texsize, vec2(x_b, y_b));

    const vec4 color = mix(texture(image0_sampler, pos_a), texture(image0_sampler, pos_b), drawable.fade);

    out_color = color * (alpha * opacity);
}
)";
};

template <>
struct ShaderSource<BuiltIn::LineSDFShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "LineSDFShader";

    static const std::array<UniformBlockInfo, 4> uniforms;
    static const std::array<AttributeInfo, 9> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

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

#if !defined(HAS_UNIFORM_u_floorwidth)
layout(location = 8) in vec2 in_floorwidth;
#endif

layout(set = 0, binding = 1) uniform LineSDFDrawableUBO {
    mat4 matrix;
    vec2 patternscale_a;
    vec2 patternscale_b;
    float ratio;
    float tex_y_a;
    float tex_y_b;
    float sdfgamma;
    float mix;
    float pad1, pad2, pad3;
} drawable;

layout(set = 0, binding = 2) uniform LineSDFInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float floorwidth_t;
    float pad1;
} interp;

layout(set = 0, binding = 4) uniform LineEvaluatedPropsUBO {
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
layout(location = 3) out vec2 frag_tex_a;
layout(location = 4) out vec2 frag_tex_b;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 5) out lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 6) out lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 7) out lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_floorwidth)
layout(location = 8) out mediump float frag_floorwidth;
#endif

void main() {

#ifndef HAS_UNIFORM_u_color
    frag_color = unpack_mix_color(in_color, interp.color_t);
#endif

#ifndef HAS_UNIFORM_u_blur
    frag_blur = unpack_mix_float(in_blur, interp.blur_t);
#endif

#ifndef HAS_UNIFORM_u_opacity
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif

#ifndef HAS_UNIFORM_u_floorwidth
    const float floorwidth = unpack_mix_float(in_floorwidth, interp.floorwidth_t);
    frag_floorwidth = floorwidth;
#else
    const float floorwidth = props.floorwidth;
#endif

#ifndef HAS_UNIFORM_u_offset
    const mediump float offset = unpack_mix_float(in_offset, interp.offset_t);
#else
    const mediump float offset = props.offset;
#endif

#ifndef HAS_UNIFORM_u_width
    const mediump float width = unpack_mix_float(in_width, interp.width_t);
#else
    const mediump float width = props.width;
#endif

#ifndef HAS_UNIFORM_u_gapwidth
    const mediump float gapwidth = unpack_mix_float(in_gapwidth, interp.gapwidth_t) / 2.0;
#else
    const mediump float gapwidth = props.gapwidth / 2.0;
#endif

    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    const float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;
    const float LINE_DISTANCE_SCALE = 2.0;

    vec2 a_extrude = in_data.xy - 128.0;
    float a_direction = mod(in_data.z, 4.0) - 1.0;
    float linesofar = (floor(in_data.z / 4.0) + in_data.w * 64.0) * LINE_DISTANCE_SCALE;
    vec2 pos = floor(in_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);
    frag_normal.y *= -1.0;

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float halfwidth = width / 2.0;

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
    mediump vec2 offset2 = offset * a_extrude * LINE_NORMAL_SCALE * frag_normal.y * mat2(t, -u, u, t);

    vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
    gl_Position = drawable.matrix * vec4(pos + offset2 / drawable.ratio, 0.0, 1.0) + projected_extrude;
    gl_Position.y *= -1.0;

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * global.units_to_pixels);
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;

    frag_width2 = vec2(outset, inset);

    frag_tex_a = vec2(linesofar * drawable.patternscale_a.x / floorwidth, (normal.y * drawable.patternscale_a.y + drawable.tex_y_a) * 2.0);
    frag_tex_b = vec2(linesofar * drawable.patternscale_b.x / floorwidth, (normal.y * drawable.patternscale_b.y + drawable.tex_y_b) * 2.0);
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in lowp vec2 frag_normal;
layout(location = 1) in lowp vec2 frag_width2;
layout(location = 2) in lowp float frag_gamma_scale;
layout(location = 3) in vec2 frag_tex_a;
layout(location = 4) in vec2 frag_tex_b;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 5) in lowp vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_blur)
layout(location = 6) in lowp float frag_blur;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 7) in lowp float frag_opacity;
#endif

#if !defined(HAS_UNIFORM_u_floorwidth)
layout(location = 8) in mediump float frag_floorwidth;
#endif

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform LineSDFDrawableUBO {
    mat4 matrix;
    vec2 patternscale_a;
    vec2 patternscale_b;
    float ratio;
    float tex_y_a;
    float tex_y_b;
    float sdfgamma;
    float mix;
    float pad1, pad2, pad3;
} drawable;

layout(set = 0, binding = 2) uniform LineSDFInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float floorwidth_t;
    float pad1;
} interp;

layout(set = 0, binding = 4) uniform LineEvaluatedPropsUBO {
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

layout(set = 1, binding = 0) uniform sampler2D image0_sampler;

void main() {

#ifdef OVERDRAW_INSPECTOR
    out_color = vec4(1.0);
    return;
#endif

#ifdef HAS_UNIFORM_u_color
    const lowp vec4 color = props.color;
#else
    const lowp vec4 color = frag_color;
#endif

#ifdef HAS_UNIFORM_u_blur
    const lowp float blur = props.blur;
#else
    const lowp float blur = frag_blur;
#endif
        
#ifdef HAS_UNIFORM_u_opacity
    const lowp float opacity = props.opacity;
#else
    const lowp float opacity = frag_opacity;
#endif

#ifdef HAS_UNIFORM_u_floorwidth
    const lowp float floorwidth = props.floorwidth;
#else
    const lowp float floorwidth = frag_floorwidth;
#endif

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (`v_width2.y`) or when fading out (`v_width2.x`)
    const float blur2 = (blur + 1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;

    const float sdfdist_a = texture(image0_sampler, frag_tex_a).a;
    const float sdfdist_b = texture(image0_sampler, frag_tex_b).a;
    const float sdfdist = mix(sdfdist_a, sdfdist_b, drawable.mix);
    const float dist = length(frag_normal) * frag_width2.x;
    const float alpha = clamp(min(dist - (frag_width2.y - blur2), frag_width2.x - dist) / blur2, 0.0, 1.0) *
                        smoothstep(0.5 - drawable.sdfgamma / floorwidth, 0.5 + drawable.sdfgamma / floorwidth, sdfdist);

    out_color = color * (alpha * opacity);
}
)";
};

} // namespace shaders
} // namespace mbgl
