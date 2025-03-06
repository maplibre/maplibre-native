#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto fillShaderPrelude = R"(

#define idFillDrawableUBO           idDrawableReservedVertexOnlyUBO
#define idFillTilePropsUBO          drawableReservedUBOCount
#define idFillEvaluatedPropsUBO     layerUBOStartId

)";

template <>
struct ShaderSource<BuiltIn::FillShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillShader";

    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 1) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 2) in vec2 in_opacity;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillDrawableUBO {
    mat4 matrix;
    // Interpolations
    float color_t;
    float opacity_t;
    float pad1;
    float pad2;
    vec4 pad3;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillDrawableUBOVector {
    FillDrawableUBO drawable_ubo[];
} drawableVector;

#if !defined(HAS_UNIFORM_u_color)
layout(location = 0) out vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) out lowp float frag_opacity;
#endif

void main() {
    const FillDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#if !defined(HAS_UNIFORM_u_color)
    frag_color = vec4(unpack_mix_color(in_color, drawable.color_t));
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();
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

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
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

    static const std::array<AttributeInfo, 3> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;

#if !defined(HAS_UNIFORM_u_outline_color)
layout(location = 1) in vec4 in_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 2) in vec2 in_opacity;
#endif

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillOutlineDrawableUBO {
    mat4 matrix;
    // Interpolations
    float outline_color_t;
    float opacity_t;
    float pad1;
    float pad2;
    vec4 pad3;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillOutlineDrawableUBOVector {
    FillOutlineDrawableUBO drawable_ubo[];
} drawableVector;

#if !defined(HAS_UNIFORM_u_outline_color)
layout(location = 0) out vec4 frag_color;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
layout(location = 1) out lowp float frag_opacity;
#endif

layout(location = 2) out vec2 frag_position;

void main() {
    const FillOutlineDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#if !defined(HAS_UNIFORM_u_outline_color)
    frag_color = vec4(unpack_mix_color(in_color, drawable.outline_color_t));
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();

    frag_position = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * paintParams.world_size;
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

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
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

    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = fillShaderPrelude;
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

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillPatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float tile_ratio;
    // Interpolations
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillPatternDrawableUBOVector {
    FillPatternDrawableUBO drawable_ubo[];
} drawableVector;

struct FillPatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float pad1;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillTilePropsUBO) readonly buffer FillPatternTilePropsUBOVector {
    FillPatternTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
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
    const FillPatternDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];
    const FillPatternTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 frag_pattern_from = tileProps.pattern_from;
#else
    frag_pattern_from = in_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 frag_pattern_to = tileProps.pattern_to;
#else
    frag_pattern_to = in_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

    const vec2 pattern_tl_a = frag_pattern_from.xy;
    const vec2 pattern_br_a = frag_pattern_from.zw;
    const vec2 pattern_tl_b = frag_pattern_to.xy;
    const vec2 pattern_br_b = frag_pattern_to.zw;

    const float pixelRatio = paintParams.pixel_ratio;
    const float tileZoomRatio = drawable.tile_ratio;
    const float fromScale = props.from_scale;
    const float toScale = props.to_scale;

    const vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    frag_pos_a = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, in_position),
    frag_pos_b = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, in_position),

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();
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

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillPatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float pad1;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillTilePropsUBO) readonly buffer FillPatternTilePropsUBOVector {
    FillPatternTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const FillPatternTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 pattern_from = tileProps.pattern_from;
#else
    const vec4 pattern_from = frag_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 pattern_to = tileProps.pattern_to;
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
    const vec2 pos = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord);
    const vec4 color1 = texture(image0_sampler, pos);

    const vec2 imagecoord_b = mod(frag_pos_b, 1.0);
    const vec2 pos2 = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    const vec4 color2 = texture(image0_sampler, pos2);

    out_color = mix(color1, color2, props.fade) * opacity;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillOutlinePatternShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillOutlinePatternShader";

    static const std::array<AttributeInfo, 4> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = fillShaderPrelude;
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

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillOutlinePatternDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float tile_ratio;
    // Interpolations
    float pattern_from_t;
    float pattern_to_t;
    float opacity_t;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillOutlinePatternDrawableUBOVector {
    FillOutlinePatternDrawableUBO drawable_ubo[];
} drawableVector;

struct FillOutlinePatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float pad1;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillTilePropsUBO) readonly buffer FillOutlinePatternTilePropsUBOVector {
    FillOutlinePatternTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
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
    const FillOutlinePatternDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];
    const FillOutlinePatternTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 frag_pattern_from = tileProps.pattern_from;
#else
    frag_pattern_from = in_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 frag_pattern_to = tileProps.pattern_to;
#else
    frag_pattern_to = in_pattern_to;
#endif

#if !defined(HAS_UNIFORM_u_opacity)
    frag_opacity = unpack_mix_float(in_opacity, drawable.opacity_t);
#endif

    const vec2 pattern_tl_a = frag_pattern_from.xy;
    const vec2 pattern_br_a = frag_pattern_from.zw;
    const vec2 pattern_tl_b = frag_pattern_to.xy;
    const vec2 pattern_br_b = frag_pattern_to.zw;

    const float pixelRatio = paintParams.pixel_ratio;
    const float tileZoomRatio = drawable.tile_ratio;
    const float fromScale = props.from_scale;
    const float toScale = props.to_scale;

    const vec2 display_size_a = vec2((pattern_br_a.x - pattern_tl_a.x) / pixelRatio, (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    const vec2 display_size_b = vec2((pattern_br_b.x - pattern_tl_b.x) / pixelRatio, (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    const vec2 position2 = in_position.xy;
    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();

    frag_pos_a = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, fromScale * display_size_a, tileZoomRatio, position2),
    frag_pos_b = get_pattern_pos(drawable.pixel_coord_upper, drawable.pixel_coord_lower, toScale * display_size_b, tileZoomRatio, position2),
    frag_pos = (gl_Position.xy / gl_Position.w + 1.0) / 2.0 * paintParams.world_size;
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

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillOutlinePatternTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float pad1;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillTilePropsUBO) readonly buffer FillOutlinePatternTilePropsUBOVector {
    FillOutlinePatternTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
    vec4 color;
    vec4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
} props;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const FillOutlinePatternTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_pattern_from)
    const vec4 pattern_from = tileProps.pattern_from;
#else
    const vec4 pattern_from = frag_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const vec4 pattern_to = tileProps.pattern_to;
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
    const vec2 pos = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord);
    const vec4 color1 = texture(image0_sampler, pos);

    const vec2 imagecoord_b = mod(frag_pos_b, 1.0);
    const vec2 pos2 = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
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

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_pos_normal;
layout(location = 1) in uvec4 in_data;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillOutlineTriangulatedDrawableUBO {
    mat4 matrix;
    float ratio;
    float pad1,
    float pad2
    float pad3;
    vec4 pad1;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillDrawableUBO) readonly buffer FillOutlineTriangulatedDrawableUBOVector {
    FillOutlineTriangulatedDrawableUBO drawable_ubo[];
} drawableVector;

layout(location = 0) out float frag_width2;
layout(location = 1) out vec2 frag_normal;
layout(location = 2) out float frag_gamma_scale;

void main() {
    const FillOutlineTriangulatedDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

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
    applySurfaceTransform();

    // calculate how much the perspective view squishes or stretches the extrude
    float extrude_length_without_perspective = length(dist);
    float extrude_length_with_perspective = length(projected_extrude.xy / gl_Position.w * paintParams.units_to_pixels);

    frag_width2 = outset;
    frag_gamma_scale = extrude_length_without_perspective / extrude_length_with_perspective;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in float frag_width2;
layout(location = 1) in vec2 frag_normal;
layout(location = 2) in float frag_gamma_scale;

layout(location = 0) out vec4 out_color;

layout(set = LAYER_SET_INDEX, binding = idFillEvaluatedPropsUBO) uniform FillEvaluatedPropsUBO {
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

} // namespace shaders
} // namespace mbgl
