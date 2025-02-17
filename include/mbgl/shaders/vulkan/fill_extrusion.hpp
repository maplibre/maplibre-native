#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto fillExtrusionShaderPrelude = R"(

#define idFillExtrusionDrawableUBO      idDrawableReservedVertexOnlyUBO
#define idFillExtrusionTilePropsUBO     drawableReservedUBOCount
#define idFillExtrusionPropsUBO         layerUBOStartId

)";

template <>
struct ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "FillExtrusionShader";

    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto prelude = fillExtrusionShaderPrelude;
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

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillExtrusionDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float height_factor;
    float tile_ratio;
    // Interpolations
    float base_t;
    float height_t;
    float color_t;
    float pattern_from_t;
    float pattern_to_t;
    float pad1;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillExtrusionDrawableUBO) readonly buffer FillExtrusionDrawableUBOVector {
    FillExtrusionDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = LAYER_SET_INDEX, binding = idFillExtrusionPropsUBO) uniform FillExtrusionPropsUBO {
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

layout(location = 0) out mediump vec4 frag_color;

void main() {
    const FillExtrusionDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_base)
    const float base = props.light_position_base.w;
#else
    const float base = max(unpack_mix_float(in_base, drawable.base_t), 0.0);
#endif

#if defined(HAS_UNIFORM_u_height)
    const float height = props.height;
#else
    const float height = max(unpack_mix_float(in_height, drawable.height_t), 0.0);
#endif

#if defined(HAS_UNIFORM_u_color)
    vec4 color = props.color;
#else
    vec4 color = unpack_mix_color(in_color, drawable.color_t);
#endif

    const vec3 normal = in_normal_ed.xyz;
    const float t = mod(normal.x, 2.0);
    const float z = t != 0.0 ? height : base;

    gl_Position = drawable.matrix * vec4(in_position, z, 1.0);
    applySurfaceTransform();

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

    static const std::array<AttributeInfo, 6> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = fillExtrusionShaderPrelude;
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

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillExtrusionDrawableUBO {
    mat4 matrix;
    vec2 pixel_coord_upper;
    vec2 pixel_coord_lower;
    float height_factor;
    float tile_ratio;
    // Interpolations
    float base_t;
    float height_t;
    float color_t;
    float pattern_from_t;
    float pattern_to_t;
    float pad1;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillExtrusionDrawableUBO) readonly buffer FillExtrusionDrawableUBOVector {
    FillExtrusionDrawableUBO drawable_ubo[];
} drawableVector;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idFillExtrusionTilePropsUBO) uniform FillExtrusionTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float pad1;
    float pad2;
} tileProps;

layout(set = LAYER_SET_INDEX, binding = idFillExtrusionPropsUBO) uniform FillExtrusionPropsUBO {
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
    const FillExtrusionDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];

#if defined(HAS_UNIFORM_u_base)
    const float base = props.light_position_base.w;
#else
    const float base = max(unpack_mix_float(in_base, drawable.base_t), 0.0);
#endif

#if defined(HAS_UNIFORM_u_height)
    const float height = props.height;
#else
    const float height = max(unpack_mix_float(in_height, drawable.height_t), 0.0);
#endif

    const vec3 normal = in_normal_ed.xyz;
    const float edgedistance = in_normal_ed.w;
    const float t = mod(normal.x, 2.0);
    const float z = t != 0.0 ? height : base;

    gl_Position = drawable.matrix * vec4(in_position, z, 1.0);
    applySurfaceTransform();

#if defined(OVERDRAW_INSPECTOR)
    frag_color = vec4(1.0);
    return;
#endif

#if defined(HAS_UNIFORM_u_pattern_from)
    const mediump vec4 pattern_from = tileProps.pattern_from;
#else
    const mediump vec4 pattern_from = in_pattern_from;
    frag_pattern_from = in_pattern_from;
#endif

#if defined(HAS_UNIFORM_u_pattern_to)
    const mediump vec4 pattern_to = tileProps.pattern_to;
#else
    const mediump vec4 pattern_to = in_pattern_to;
    frag_pattern_to = in_pattern_to;
#endif

    const vec2 pattern_tl_a = pattern_from.xy;
    const vec2 pattern_br_a = pattern_from.zw;
    const vec2 pattern_tl_b = pattern_to.xy;
    const vec2 pattern_br_b = pattern_to.zw;

    const float pixelRatio = paintParams.pixel_ratio;
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

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

struct FillExtrusionTilePropsUBO {
    vec4 pattern_from;
    vec4 pattern_to;
    vec2 texsize;
    float pad1;
    float pad2;
};

layout(std140, set = LAYER_SET_INDEX, binding = idFillExtrusionTilePropsUBO) readonly buffer FillExtrusionTilePropsUBOVector {
    FillExtrusionTilePropsUBO tile_props_ubo[];
} tilePropsVector;

layout(set = LAYER_SET_INDEX, binding = idFillExtrusionPropsUBO) uniform FillExtrusionPropsUBO {
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

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const FillExtrusionTilePropsUBO tileProps = tilePropsVector.tile_props_ubo[constant.ubo_index];

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

    out_color = mix(color1, color2, props.fade) * frag_lighting;
}
)";
};

} // namespace shaders
} // namespace mbgl
