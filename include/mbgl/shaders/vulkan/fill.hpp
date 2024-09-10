#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillShader";

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 1) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 2) in vec2 in_opacity;
#endif

layout(set = 0, binding = 1) uniform FillDrawableUBO {
    mat4 matrix;
} drawable;

layout(set = 0, binding = 3) uniform FillInterpolateUBO {
    float color_t;
    float opacity_t;
} interp;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 0) out vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) out lowp float frag_opacity;
#endif

void main() {

#if !defined(HAS_UNIFORM_u_color)
    frag_color = vec4(unpack_mix_color(in_color, interp.color_t));
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    gl_Position.y *= -1.0;
}
)";

    static constexpr auto fragment = R"(

#if !defined(HAS_UNIFORM_u_color)
layout(location = 0) in vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) in lowp float frag_opacity;
#endif

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

void main() {

#if defined(HAS_UNIFORM_u_color)
    const vec4 color = props.color;
#else
    const vec4 color = frag_color;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = frag_opacity;
#endif

    out_color = color * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillOutlineShader";

    static const std::array<UniformBlockInfo, 4> uniforms;
    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_outline_color)
layout(location = 1) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 2) in vec2 in_opacity;
#endif

layout(set = 0, binding = 1) uniform FillDrawableUBO {
    mat4 matrix;
} drawable;

layout(set = 0, binding = 3) uniform FillInterpolateUBO {
    float color_t;
    float opacity_t;
} interp;

#if !defined(HAS_UNIFORM_u_outline_color)
layout(location = 0) out vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) out lowp float frag_opacity;
#endif

layout(location = 2) out vec2 frag_position;

void main() {

#if !defined(HAS_UNIFORM_u_outline_color)
    frag_color = vec4(unpack_mix_color(in_color, interp.color_t));
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    gl_Position.y *= -1.0;

    frag_position = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * global.world_size;
}
)";

    static constexpr auto fragment = R"(

#if !defined(HAS_UNIFORM_u_outline_color)
layout(location = 0) in vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) in float frag_opacity;
#endif

layout(location = 2) in vec2 frag_position;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_outline_color)
    const vec4 color = props.outline_color;
#else
    const vec4 color = frag_color;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = frag_opacity;
#endif

    float dist = length(frag_position - gl_FragCoord.xy);
    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    out_color = color * opacity * alpha;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillPatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillPatternShader";

    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 1) in mediump uvec4 in_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 2) in mediump uvec4 in_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 3) in vec2 in_opacity;
#endif

layout(set = 0, binding = 1) uniform FillPatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    vec2 texsize;
    float tile_ratio;
    float pad;
} drawable;

layout(set = 0, binding = 2) uniform FillPatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
} tile;

layout(set = 0, binding = 3) uniform FillPatternInterpolateUBO {
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
} interp;

layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

layout(location = 0) out vec2 frag_pos_a;
layout(location = 1) out vec2 frag_pos_b;

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 2) out mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 3) out mediump vec4 frag_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 4) out lowp float frag_opacity;
#endif

void main() {

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 frag_pattern_from = tile.pattern_from;
#else
    frag_pattern_from = in_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 frag_pattern_to = tile.pattern_to;
#else
    frag_pattern_to = in_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif

    const vec2 pattern_tl_a = frag_pattern_from.xy; 
    const vec2 pattern_br_a = frag_pattern_from.zw; 
    const vec2 pattern_tl_b = frag_pattern_to.xy; 
    const vec2 pattern_br_b = frag_pattern_to.zw;

    const float pixelRatio = global.pixel_ratio;
    const float tileZoomRatio = drawable.tile_ratio;
    const float fromScale = props.from_scale;
    const float toScale = props.to_scale;

    const vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    frag_pos_a = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, in_position),
    frag_pos_b = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, in_position),

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    gl_Position.y *= -1.0;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_pos_a;
layout(location = 1) in vec2 frag_pos_b;

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 2) in vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 3) in vec4 frag_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 4) in lowp float frag_opacity;
#endif

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform FillPatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    vec2 texsize;
    float tile_ratio;
    float pad;
} drawable;

layout(set = 0, binding = 2) uniform FillPatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
} tile;

layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

layout(set = 1, binding = 0) uniform sampler2D image0_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 pattern_from = tile.pattern_from;
#else
    const vec4 pattern_from = frag_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 pattern_to = tile.pattern_to;
#else
    const vec4 pattern_to = frag_pattern_to;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = frag_opacity;
#endif

    const vec2 pattern_tl_a = pattern_from.xy;
    const vec2 pattern_br_a = pattern_from.zw;
    const vec2 pattern_tl_b = pattern_to.xy;
    const vec2 pattern_br_b = pattern_to.zw;

    const vec2 imagecoord = mod(frag_pos_a, 1.0);
    const vec2 pos = mix(pattern_tl_a / drawable.texsize, pattern_br_a / drawable.texsize, imagecoord);
    const vec4 color1 = texture(image0_sampler, pos);

    const vec2 imagecoord_b = mod(frag_pos_b, 1.0);
    const vec2 pos2 = mix(pattern_tl_b / drawable.texsize, pattern_br_b / drawable.texsize, imagecoord_b);
    const vec4 color2 = texture(image0_sampler, pos2);

    out_color = mix(color1, color2, props.fade) * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillOutlinePatternShader";

    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 1) in mediump uvec4 in_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 2) in mediump uvec4 in_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 3) in vec2 in_opacity;
#endif

layout(set = 0, binding = 1) uniform FillOutlinePatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    vec2 texsize;
    float tile_ratio;
    float pad;
} drawable;

layout(set = 0, binding = 2) uniform FillOutlinePatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
} tile;

layout(set = 0, binding = 3) uniform FillOutlinePatternInterpolateUBO {
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
} interp;

layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

layout(location = 0) out vec2 frag_pos_a;
layout(location = 1) out vec2 frag_pos_b;
layout(location = 2) out vec2 frag_pos;

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 3) out mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 4) out mediump vec4 frag_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) out lowp float frag_opacity;
#endif

void main() {

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 frag_pattern_from = tile.pattern_from;
#else
    frag_pattern_from = in_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 frag_pattern_to = tile.pattern_to;
#else
    frag_pattern_to = in_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, interp.opacity_t);
#endif

    const vec2 pattern_tl_a = frag_pattern_from.xy; 
    const vec2 pattern_br_a = frag_pattern_from.zw; 
    const vec2 pattern_tl_b = frag_pattern_to.xy; 
    const vec2 pattern_br_b = frag_pattern_to.zw;

    const float pixelRatio = global.pixel_ratio;
    const float tileZoomRatio = drawable.tile_ratio;
    const float fromScale = props.from_scale;
    const float toScale = props.to_scale;

    const vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    const vec2 position2 = in_position.xy;
    vec4 position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    position.y *= -1.0;

    frag_pos_a = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, position2),
    frag_pos_b = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, position2),
    frag_pos = (position.xy / position.w + 1.0) / 2.0 * global.world_size;
    
    gl_Position = position;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_pos_a;
layout(location = 1) in vec2 frag_pos_b;
layout(location = 2) in vec2 frag_pos;

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 3) in vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 4) in vec4 frag_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 5) in lowp float frag_opacity;
#endif

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform FillOutlinePatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    vec2 texsize;
    float tile_ratio;
    float pad;
} drawable;

layout(set = 0, binding = 2) uniform FillOutlinePatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
} tile;

layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

layout(set = 1, binding = 0) uniform sampler2D image0_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 pattern_from = tile.pattern_from;
#else
    const vec4 pattern_from = frag_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 pattern_to = tile.pattern_to;
#else
    const vec4 pattern_to = frag_pattern_to;
#endif

#if defined(HAS_UNIFORM_u_opacity)
    const float opacity = props.opacity;
#else
    const float opacity = frag_opacity;
#endif

    const vec2 pattern_tl_a = pattern_from.xy;
    const vec2 pattern_br_a = pattern_from.zw;
    const vec2 pattern_tl_b = pattern_to.xy;
    const vec2 pattern_br_b = pattern_to.zw;

    const vec2 imagecoord = mod(frag_pos_a, 1.0);
    const vec2 pos = mix(pattern_tl_a / drawable.texsize, pattern_br_a / drawable.texsize, imagecoord);
    const vec4 color1 = texture(image0_sampler, pos);

    const vec2 imagecoord_b = mod(frag_pos_b, 1.0);
    const vec2 pos2 = mix(pattern_tl_b / drawable.texsize, pattern_br_b / drawable.texsize, imagecoord_b);
    const vec4 color2 = texture(image0_sampler, pos2);

    const float dist = length(frag_pos - gl_FragCoord.xy);
    const float alpha = 1.0 - smoothstep(0.0, 1.0, dist);

    out_color = mix(color1, color2, props.fade) * alpha * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlineTriangulatedShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillOutlineTriangulatedShader";

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

layout(set = 0, binding = 1) uniform FillOutlineTriangulatedDrawableUBO {
    mat4 matrix;
    float ratio;
    float pad1, pad2, pad3;
} drawable;

layout(location = 0) out float frag_width2;
layout(location = 1) out vec2 frag_normal;
layout(location = 2) out float frag_gamma_scale;

void main() {
    // the distance over which the line edge fades out.
    // Retina devices need a smaller distance to avoid aliasing.
    float ANTIALIASING = 1.0 / DEVICE_PIXEL_RATIO / 2.0;

    const vec2 a_extrude = in_data.xy - 128.0;
    const vec2 pos = floor(in_pos_normal * 0.5);

    // x is 1 if it's a round cap, 0 otherwise
    // y is 1 if the normal points up, and -1 if it points down
    // We store these in the least significant bit of in_pos_normal
    mediump vec2 normal = in_pos_normal - 2.0 * pos;
    frag_normal = vec2(normal.x, normal.y * 2.0 - 1.0);
    frag_normal.y *= -1.0;

    // these transformations used to be applied in the JS and native code bases.
    // moved them into the shader for clarity and simplicity.
    float width = 1.0;
    float halfwidth = width / 2.0;
    float outset = halfwidth + (halfwidth == 0.0 ? 0.0 : ANTIALIASING);

    // Scale the extrusion vector down to a normal and then up by the line width
    // of this vertex.
    mediump vec2 dist = outset * a_extrude * LINE_NORMAL_SCALE;

    vec4 projected_extrude = drawable.matrix * vec4(dist / drawable.ratio, 0.0, 0.0);
    gl_Position = drawable.matrix * vec4(pos, 0.0, 1.0) + projected_extrude;
    gl_Position.y *= -1.0;

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * global.units_to_pixels);

    frag_width2 = outset;
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in float frag_width2;
layout(location = 1) in vec2 frag_normal;
layout(location = 2) in float frag_gamma_scale;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 4) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    // Calculate the distance of the pixel from the line in pixels.
    const float dist = length(frag_normal) * frag_width2;

    // Calculate the antialiasing fade factor. This is either when fading in the
    // line in case of an offset line (`v_width2.y`) or when fading out (`v_width2.x`)
    const float blur2 = (1.0 / DEVICE_PIXEL_RATIO) * frag_gamma_scale;
    const float alpha = clamp(min(dist + blur2, frag_width2 - dist) / blur2, 0.0, 1.0);

    out_color = props.outline_color * alpha * props.opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillExtrusionShader";

    static const std::array<UniformBlockInfo, 3> uniforms;
    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static constexpr std::array<TextureInfo, 0> textures{};

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec4 in_normal_ed;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 2) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_base)
layout(location = 3) in vec2 in_base;
#endif

#if !defined(HAS_UNIFORM_u_height)
layout(location = 4) in vec2 in_height;
#endif

layout(set = 0, binding = 1) uniform FillExtrusionDrawableUBO {
    mat4 matrix;
    vec2 texsize;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float height_factor;
    float tile_ratio;
} drawable;

layout(set = 0, binding = 2) uniform FillExtrusionPropsUBO {
    vec4 color;
    vec4 light_color_pad;
    vec4 light_position_base;
    float height;
    float light_intensity;
    float vertical_gradient;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
    float pad2;
} props;

layout(set = 0, binding = 4) uniform FillExtrusionInterpolateUBO {
    float base_t;
    float height_t;
    float color_t;
    float pattern_from_t;
    float pattern_to_t;
    float pad1, pad2, pad3;
} interp;

layout(location = 0) out mediump vec4 frag_color;

void main() {

#if defined(HAS_UNIFORM_u_base)
    const float base = props.light_position_base.w;
#else
    const float base = max(unpack_mix_float(in_base, interp.base_t), 0.0);
#endif

#if defined(HAS_UNIFORM_u_height)
    const float height = props.height;
#else
    const float height = max(unpack_mix_float(in_height, interp.height_t), 0.0);
#endif

#if defined(HAS_UNIFORM_u_color)
    vec4 color = props.color;
#else
    vec4 color = unpack_mix_color(in_color, interp.color_t);
#endif

    const vec3 normal = in_normal_ed.xyz;
    const float t = mod(normal.x, 2.0);
    const float z = t != 0.0 ? height : base;

    gl_Position = drawable.matrix * vec4(in_position, z, 1.0);
    gl_Position.y *= -1.0;

#if defined(OVERDRAW_INSPECTOR)
    frag_color = vec4(1.0);
    return;
#endif

    // Relative luminance (how dark/bright is the surface color?)
    const float luminance = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;

    vec4 vcolor = vec4(0.0, 0.0, 0.0, 1.0);

    // Add slight ambient lighting so no extrusions are totally black
    color += vec4(0.03, 0.03, 0.03, 1.0);

    // Calculate cos(theta), where theta is the angle between surface normal and diffuse light ray
    const float directionalFraction = clamp(dot(normal / 16384.0, props.light_position_base.xyz), 0.0, 1.0);

    // Adjust directional so that the range of values for highlight/shading is
    // narrower with lower light intensity and with lighter/brighter surface colors
    const float minDirectional = 1.0 - props.light_intensity;
    const float maxDirectional = max(1.0 - luminance + props.light_intensity, 1.0);
    float directional = mix(minDirectional, maxDirectional, directionalFraction);

    // Add gradient along z axis of side surfaces
    if (normal.y != 0.0) {
        // This avoids another branching statement, but multiplies by a constant of 0.84 if no
        // vertical gradient, and otherwise calculates the gradient based on base + height
        // TODO: If we're optimizing to the level of avoiding branches, we should pre-compute
        //       the square root when height is a uniform.
        const float fMin = mix(0.7, 0.98, 1.0 - props.light_intensity);
        const float factor = clamp((t + base) * pow(height / 150.0, 0.5), fMin, 1.0);
        directional *= (1.0 - props.vertical_gradient) + (props.vertical_gradient * factor);
    }

    // Assign final color based on surface + ambient light color, diffuse light directional,
    // and light color with lower bounds adjusted to hue of light so that shading is tinted
    // with the complementary (opposite) color to the light color
    const vec3 light_color = props.light_color_pad.rgb;
    const vec3 minLight = mix(vec3(0.0), vec3(0.3), 1.0 - light_color.rgb);
    vcolor += vec4(clamp(color.rgb * directional * light_color.rgb, minLight, vec3(1.0)), 0.0);

    frag_color = vcolor * props.opacity;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec4 frag_color;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = frag_color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillExtrusionPatternShader";

    static const std::array<UniformBlockInfo, 5> uniforms;
    static const std::array<AttributeInfo, 6> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec4 in_normal_ed;

#if !defined(HAS_UNIFORM_u_base)
layout(location = 2) in vec2 in_base;
#endif

#if !defined(HAS_UNIFORM_u_height)
layout(location = 3) in vec2 in_height;
#endif

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 4) in uvec4 in_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 5) in uvec4 in_pattern_to;
#endif

layout(set = 0, binding = 1) uniform FillExtrusionDrawableUBO {
    mat4 matrix;
    vec2 texsize;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float height_factor;
    float tile_ratio;
} drawable;

layout(set = 0, binding = 2) uniform FillExtrusionPropsUBO {
    vec4 color;
    vec4 light_color_pad;
    vec4 light_position_base;
    float height;
    float light_intensity;
    float vertical_gradient;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
    float pad2;
} props;

layout(set = 0, binding = 3) uniform FillExtrusionTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
} tile;

layout(set = 0, binding = 4) uniform FillExtrusionInterpolateUBO {
    float base_t;
    float height_t;
    float color_t;
    float pattern_from_t;
    float pattern_to_t;
    float pad1, pad2, pad3;
} interp;

layout(location = 0) out mediump vec4 frag_lighting;
layout(location = 1) out mediump vec2 frag_pos_a;
layout(location = 2) out mediump vec2 frag_pos_b;

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 3) out mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 4) out mediump vec4 frag_pattern_to;
#endif

void main() {

#if defined(HAS_UNIFORM_u_base)
    const float base = props.light_position_base.w;
#else
    const float base = max(unpack_mix_float(in_base, interp.base_t), 0.0);
#endif

#if defined(HAS_UNIFORM_u_height)
    const float height = props.height;
#else
    const float height = max(unpack_mix_float(in_height, interp.height_t), 0.0);
#endif

    const vec3 normal = in_normal_ed.xyz;
    const float edgedistance = in_normal_ed.w;
    const float t = mod(normal.x, 2.0);
    const float z = t != 0.0 ? height : base;

    gl_Position = drawable.matrix * vec4(in_position, z, 1.0);
    gl_Position.y *= -1.0;

#if defined(OVERDRAW_INSPECTOR)
    frag_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const mediump vec4 pattern_from = tile.pattern_from;
#else
    const mediump vec4 pattern_from = in_pattern_from;
    frag_pattern_from = in_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const mediump vec4 pattern_to = tile.pattern_to;
#else
    const mediump vec4 pattern_to = in_pattern_to;
    frag_pattern_to = in_pattern_to;
#endif

    const vec2 pattern_tl_a = pattern_from.xy;
    const vec2 pattern_br_a = pattern_from.zw;
    const vec2 pattern_tl_b = pattern_to.xy;
    const vec2 pattern_br_b = pattern_to.zw;

    const float pixelRatio = global.pixel_ratio;
    const float tileZoomRatio = drawable.tile_ratio;
    const float fromScale = props.from_scale;
    const float toScale = props.to_scale;

    const vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    const vec2 pos = normal.x == 1.0 && normal.y == 0.0 && normal.z == 16384.0
        ? vec2(in_position) // extrusion top
        : vec2(edgedistance, z * drawable.height_factor); // extrusion side
    
    vec4 lighting = vec4(0.0, 0.0, 0.0, 1.0);
    float directional = clamp(dot(normal / 16383.0, props.light_position_base.xyz), 0.0, 1.0);
    directional = mix((1.0 - props.light_intensity), max((0.5 + props.light_intensity), 1.0), directional);

    if (normal.y != 0.0) {
        // This avoids another branching statement, but multiplies by a constant of 0.84 if no vertical gradient,
        // and otherwise calculates the gradient based on base + height
        directional *= (
            (1.0 - props.vertical_gradient) +
            (props.vertical_gradient * clamp((t + base) * pow(height / 150.0, 0.5), mix(0.7, 0.98, 1.0 - props.light_intensity), 1.0)));
    }

    lighting.rgb += clamp(directional * props.light_color_pad.rgb, mix(vec3(0.0), vec3(0.3), 1.0 - props.light_color_pad.rgb), vec3(1.0));
    lighting *= props.opacity;

    frag_lighting = lighting;
    frag_pos_a = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, pos);
    frag_pos_b = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, pos);

#if !defined(HAS_UNIFORM_u_pattern_from)
    frag_pattern_from = in_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
    frag_pattern_to = in_pattern_to;
#endif
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in mediump vec4 frag_lighting;
layout(location = 1) in mediump vec2 frag_pos_a;
layout(location = 2) in mediump vec2 frag_pos_b;

#if !defined(HAS_UNIFORM_u_pattern_from)
layout(location = 3) in mediump vec4 frag_pattern_from;
#endif

#if !defined(HAS_UNIFORM_u_pattern_to)
layout(location = 4) in mediump vec4 frag_pattern_to;
#endif

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform FillExtrusionDrawableUBO {
    mat4 matrix;
    vec2 texsize;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float height_factor;
    float tile_ratio;
} drawable;

layout(set = 0, binding = 2) uniform FillExtrusionPropsUBO {
    vec4 color;
    vec4 light_color_pad;
    vec4 light_position_base;
    float height;
    float light_intensity;
    float vertical_gradient;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
    float pad2;
} props;

layout(set = 0, binding = 3) uniform FillExtrusionTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
} tile;

layout(set = 1, binding = 0) uniform sampler2D image0_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 pattern_from = tile.pattern_from;
#else
    const vec4 pattern_from = frag_pattern_from;
#endif
#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 pattern_to = tile.pattern_to;
#else
    const vec4 pattern_to = frag_pattern_to;
#endif

    const vec2 pattern_tl_a = pattern_from.xy;
    const vec2 pattern_br_a = pattern_from.zw;
    const vec2 pattern_tl_b = pattern_to.xy;
    const vec2 pattern_br_b = pattern_to.zw;

    const vec2 imagecoord = mod(frag_pos_a, 1.0);
    const vec2 pos = mix(pattern_tl_a / drawable.texsize, pattern_br_a / drawable.texsize, imagecoord);
    const vec4 color1 = texture(image0_sampler, pos);

    const vec2 imagecoord_b = mod(frag_pos_b, 1.0);
    const vec2 pos2 = mix(pattern_tl_b / drawable.texsize, pattern_br_b / drawable.texsize, imagecoord_b);
    const vec4 color2 = texture(image0_sampler, pos2);

    out_color = mix(color1, color2, props.fade) * frag_lighting;
}
)";
};

} // namespace shaders
} // namespace mbgl
