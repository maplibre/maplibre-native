#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::FillExtrusionShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillExtrusionShader";
    static const std::array<AttributeInfo, 5> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 0> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec2<i32>,
    @location(4) normal_ed: vec4<i32>,
#ifndef HAS_UNIFORM_u_color
    @location(5) color: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_base
    @location(6) base: vec2<f32>,
#endif
#ifndef HAS_UNIFORM_u_height
    @location(7) height: vec2<f32>,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
};

struct FillExtrusionDrawableUBO {
    matrix: mat4x4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    height_factor: f32,
    tile_ratio: f32,
    base_t: f32,
    height_t: f32,
    color_t: f32,
    pattern_from_t: f32,
    pattern_to_t: f32,
    pad1: f32,
};

struct FillExtrusionPropsUBO {
    color: vec4<f32>,
    light_color_pad: vec4<f32>,
    light_position_base: vec4<f32>,
    height: f32,
    light_intensity: f32,
    vertical_gradient: f32,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
    pad2: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(2) var<storage, read> drawableVector: array<FillExtrusionDrawableUBO>;
@group(0) @binding(5) var<uniform> props: FillExtrusionPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let drawable = drawableVector[globalIndex.value];

#ifndef HAS_UNIFORM_u_base
    let baseValue = max(unpack_mix_float(in.base, drawable.base_t), 0.0);
#else
    let baseValue = max(props.light_position_base.w, 0.0);
#endif

#ifndef HAS_UNIFORM_u_height
    let heightValue = max(unpack_mix_float(in.height, drawable.height_t), 0.0);
#else
    let heightValue = max(props.height, 0.0);
#endif

    let normal_i = vec3<f32>(f32(in.normal_ed.x), f32(in.normal_ed.y), f32(in.normal_ed.z));
    let t = glMod(normal_i.x, 2.0);
    let z = select(baseValue, heightValue, t != 0.0);

    out.position = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), z, 1.0);

#ifdef OVERDRAW_INSPECTOR
    out.color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
    return out;
#endif

#ifndef HAS_UNIFORM_u_color
    var color = unpack_mix_color(in.color, drawable.color_t);
#else
    var color = props.color;
#endif
    color = color + min(vec4<f32>(0.03, 0.03, 0.03, 1.0), vec4<f32>(1.0));

    let luminance = dot(color.rgb, vec3<f32>(0.2126, 0.7152, 0.0722));
    let unitNormal = normal_i / 16384.0;
    let directionalFraction = clamp(dot(unitNormal, props.light_position_base.xyz), 0.0, 1.0);
    let minDirectional = 1.0 - props.light_intensity;
    let maxDirectional = max(1.0 - luminance + props.light_intensity, 1.0);
    var directional = mix(minDirectional, maxDirectional, directionalFraction);

    if (normal_i.y != 0.0) {
        let gradientMin = mix(0.7, 0.98, 1.0 - props.light_intensity);
        let factor = clamp((t + baseValue) * pow(heightValue / 150.0, 0.5), gradientMin, 1.0);
        directional *= (1.0 - props.vertical_gradient) + props.vertical_gradient * factor;
    }

    let lightColor = props.light_color_pad.xyz;
    let minLight = mix(vec3<f32>(0.0), vec3<f32>(0.3), 1.0 - lightColor);
    let lit = clamp(color.rgb * directional * lightColor, minLight, vec3<f32>(1.0));

    var vcolor = vec4<f32>(0.0, 0.0, 0.0, 1.0);
    vcolor += vec4<f32>(lit, 0.0);

    out.color = vcolor * props.opacity;
    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) color: vec4<f32>,
};

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    return in.color;
}
)";
};

template <>
struct ShaderSource<BuiltIn::FillExtrusionPatternShader, gfx::Backend::Type::WebGPU> {
    static constexpr const char* name = "FillExtrusionPatternShader";
    static const std::array<AttributeInfo, 6> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(
struct VertexInput {
    @location(3) position: vec2<i32>,
    @location(4) normal_ed: vec4<i32>,
#ifndef HAS_UNIFORM_u_base
    @location(5) base: vec2<f32>,
#endif
#ifndef HAS_UNIFORM_u_height
    @location(6) height: vec2<f32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_from
    @location(7) pattern_from: vec4<u32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    @location(8) pattern_to: vec4<u32>,
#endif
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) lighting: vec4<f32>,
    @location(1) pos_a: vec2<f32>,
    @location(2) pos_b: vec2<f32>,
#ifndef HAS_UNIFORM_u_pattern_from
    @location(3) pattern_from: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    @location(4) pattern_to: vec4<f32>,
#endif
};

struct FillExtrusionPatternDrawableUBO {
    matrix: mat4x4<f32>,
    pixel_coord_upper: vec2<f32>,
    pixel_coord_lower: vec2<f32>,
    height_factor: f32,
    tile_ratio: f32,
    base_t: f32,
    height_t: f32,
    pattern_from_t: f32,
    pattern_to_t: f32,
    pad1: f32,
    pad2: f32,
};

struct FillExtrusionPatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
};

struct FillExtrusionPropsUBO {
    color: vec4<f32>,
    light_color_pad: vec4<f32>,
    light_position_base: vec4<f32>,
    height: f32,
    light_intensity: f32,
    vertical_gradient: f32,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
    pad2: f32,
};

struct GlobalPaintParamsUBO {
    pixel_ratio: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(0) var<uniform> paintParams: GlobalPaintParamsUBO;
@group(0) @binding(2) var<storage, read> drawableVector: array<FillExtrusionPatternDrawableUBO>;
@group(0) @binding(4) var<storage, read> tilePropsVector: array<FillExtrusionPatternTilePropsUBO>;
@group(0) @binding(5) var<uniform> props: FillExtrusionPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let index = globalIndex.value;
    let drawable = drawableVector[index];

#ifndef HAS_UNIFORM_u_base
    let baseValue = max(unpack_mix_float(in.base, drawable.base_t), 0.0);
#else
    let baseValue = max(props.light_position_base.w, 0.0);
#endif

#ifndef HAS_UNIFORM_u_height
    let heightValue = max(unpack_mix_float(in.height, drawable.height_t), 0.0);
#else
    let heightValue = max(props.height, 0.0);
#endif

    let normal_i = vec3<f32>(f32(in.normal_ed.x), f32(in.normal_ed.y), f32(in.normal_ed.z));
    let edgedistance = f32(in.normal_ed.w);
    let t = glMod(normal_i.x, 2.0);
    let z = select(baseValue, heightValue, t != 0.0);

    out.position = drawable.matrix * vec4<f32>(f32(in.position.x), f32(in.position.y), z, 1.0);

    var patternPos: vec2<f32>;
    if (normal_i.x == 1.0 && normal_i.y == 0.0 && normal_i.z == 16384.0) {
        patternPos = vec2<f32>(f32(in.position.x), f32(in.position.y));
    } else {
        patternPos = vec2<f32>(edgedistance, z * drawable.height_factor);
    }

    let pixelRatio = paintParams.pixel_ratio;
    let tileZoomRatio = drawable.tile_ratio;
    let fromScale = props.from_scale;
    let toScale = props.to_scale;

#ifdef HAS_UNIFORM_u_pattern_from
    let patternFrom = tilePropsVector[index].pattern_from;
#else
    let patternFrom = vec4<f32>(in.pattern_from);
    out.pattern_from = patternFrom;
#endif

#ifdef HAS_UNIFORM_u_pattern_to
    let patternTo = tilePropsVector[index].pattern_to;
#else
    let patternTo = vec4<f32>(in.pattern_to);
    out.pattern_to = patternTo;
#endif

    let pattern_tl_a = patternFrom.xy;
    let pattern_br_a = patternFrom.zw;
    let pattern_tl_b = patternTo.xy;
    let pattern_br_b = patternTo.zw;

    let display_size_a = vec2<f32>((pattern_br_a.x - pattern_tl_a.x) / pixelRatio,
                                   (pattern_br_a.y - pattern_tl_a.y) / pixelRatio);
    let display_size_b = vec2<f32>((pattern_br_b.x - pattern_tl_b.x) / pixelRatio,
                                   (pattern_br_b.y - pattern_tl_b.y) / pixelRatio);

    var lighting = vec4<f32>(0.0, 0.0, 0.0, 1.0);
    var directional = clamp(dot(normal_i / 16383.0, props.light_position_base.xyz), 0.0, 1.0);
    directional = mix(1.0 - props.light_intensity, max(0.5 + props.light_intensity, 1.0), directional);

    if (normal_i.y != 0.0) {
        let gradientMin = mix(0.7, 0.98, 1.0 - props.light_intensity);
        let factor = clamp((t + baseValue) * pow(heightValue / 150.0, 0.5), gradientMin, 1.0);
        directional *= (1.0 - props.vertical_gradient) + props.vertical_gradient * factor;
    }

    let lightColor = props.light_color_pad.xyz;
    let lit = clamp(directional * lightColor,
                    mix(vec3<f32>(0.0), vec3<f32>(0.3), 1.0 - lightColor),
                    vec3<f32>(1.0));
    lighting = vec4<f32>(lighting.rgb + lit, lighting.a);
    lighting *= props.opacity;

    out.lighting = lighting;
    out.pos_a = get_pattern_pos(drawable.pixel_coord_upper,
                                drawable.pixel_coord_lower,
                                fromScale * display_size_a,
                                tileZoomRatio,
                                patternPos);
    out.pos_b = get_pattern_pos(drawable.pixel_coord_upper,
                                drawable.pixel_coord_lower,
                                toScale * display_size_b,
                                tileZoomRatio,
                                patternPos);

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) lighting: vec4<f32>,
    @location(1) pos_a: vec2<f32>,
    @location(2) pos_b: vec2<f32>,
#ifndef HAS_UNIFORM_u_pattern_from
    @location(3) pattern_from: vec4<f32>,
#endif
#ifndef HAS_UNIFORM_u_pattern_to
    @location(4) pattern_to: vec4<f32>,
#endif
};

struct FillExtrusionPatternTilePropsUBO {
    pattern_from: vec4<f32>,
    pattern_to: vec4<f32>,
    texsize: vec2<f32>,
    pad1: f32,
    pad2: f32,
};

struct FillExtrusionPropsUBO {
    color: vec4<f32>,
    light_color_pad: vec4<f32>,
    light_position_base: vec4<f32>,
    height: f32,
    light_intensity: f32,
    vertical_gradient: f32,
    opacity: f32,
    fade: f32,
    from_scale: f32,
    to_scale: f32,
    pad2: f32,
};

struct GlobalIndexUBO {
    value: u32,
    pad0: vec3<u32>,
};

@group(0) @binding(4) var<storage, read> tilePropsVector: array<FillExtrusionPatternTilePropsUBO>;
@group(0) @binding(5) var<uniform> props: FillExtrusionPropsUBO;
@group(0) @binding(1) var<uniform> globalIndex: GlobalIndexUBO;
@group(1) @binding(0) var texture_sampler: sampler;
@group(1) @binding(1) var pattern_texture: texture_2d<f32>;

@fragment
fn main(in: FragmentInput) -> @location(0) vec4<f32> {
    let tileProps = tilePropsVector[globalIndex.value];

#ifdef HAS_UNIFORM_u_pattern_from
    let pattern_from = tileProps.pattern_from;
#else
    let pattern_from = in.pattern_from;
#endif

#ifdef HAS_UNIFORM_u_pattern_to
    let pattern_to = tileProps.pattern_to;
#else
    let pattern_to = in.pattern_to;
#endif

    let pattern_tl_a = pattern_from.xy;
    let pattern_br_a = pattern_from.zw;
    let pattern_tl_b = pattern_to.xy;
    let pattern_br_b = pattern_to.zw;

    let imagecoord_a = gl_mod(in.pos_a, vec2<f32>(1.0));
    let pos_a = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord_a);
    let color_a = textureSample(pattern_texture, texture_sampler, pos_a);

    let imagecoord_b = gl_mod(in.pos_b, vec2<f32>(1.0));
    let pos_b = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    let color_b = textureSample(pattern_texture, texture_sampler, pos_b);

    let pattern_color = mix(color_a, color_b, props.fade);
    return pattern_color * in.lighting;
}
)";
};

} // namespace shaders
} // namespace mbgl
