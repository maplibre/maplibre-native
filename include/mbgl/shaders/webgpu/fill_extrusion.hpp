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
    @location(5) color: vec4<f32>,
    @location(6) base: f32,
    @location(7) height: f32,
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
    
    let base = max(unpack_mix_float(vec2<f32>(in.base, in.base), drawable.base_t), 0.0);
    let height = max(unpack_mix_float(vec2<f32>(in.height, in.height), drawable.height_t), 0.0);
    
    let normal = vec3<f32>(f32(in.normal_ed.x), f32(in.normal_ed.y), f32(in.normal_ed.z));
    let t = mix(base, height, f32(in.normal_ed.w));
    
    let pos = vec3<f32>(f32(in.position.x), f32(in.position.y), t);
    
    out.position = drawable.matrix * vec4<f32>(pos, 1.0);
    
    // Calculate lighting
    let light_position = props.light_position_base.xyz;
    let light_color = props.light_color_pad.xyz;
    let light_intensity = props.light_intensity;
    
    // Simple directional lighting
    let light_dir = normalize(light_position);
    let n_dot_l = max(dot(normal, light_dir), 0.0);
    
    let base_color = unpack_mix_color(in.color, drawable.color_t);
    let lit_color = base_color * vec4<f32>(light_color * light_intensity * n_dot_l, 1.0);
    
    // Apply vertical gradient
    let vertical_gradient = props.vertical_gradient;
    let gradient_factor = mix(1.0, 0.5, vertical_gradient * (height - base) / (props.height));
    
    out.color = vec4<f32>(lit_color.rgb * gradient_factor, base_color.a * props.opacity);
    
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
    @location(5) base: f32,
    @location(6) height: f32,
    @location(7) pattern_from: vec4<u32>,
    @location(8) pattern_to: vec4<u32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) lighting: vec4<f32>,
    @location(1) pos_a: vec2<f32>,
    @location(2) pos_b: vec2<f32>,
    @location(3) pattern_from: vec4<f32>,
    @location(4) pattern_to: vec4<f32>,
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
    // other fields...
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
    let tileProps = tilePropsVector[index];

    // Unpack base and height using helper functions
    let base = max(unpack_mix_float(vec2<f32>(in.base, in.base), drawable.base_t), 0.0);
    let height = max(unpack_mix_float(vec2<f32>(in.height, in.height), drawable.height_t), 0.0);

    // Process normal and edge distance
    let normal = vec3<f32>(f32(in.normal_ed.x), f32(in.normal_ed.y), f32(in.normal_ed.z));
    let edgedistance = f32(in.normal_ed.w);
    let t = mod(normal.x, 2.0);
    let z = select(base, height, t != 0.0);

    let position = drawable.matrix * vec4<f32>(vec2<f32>(f32(in.position.x), f32(in.position.y)), z, 1.0);

    // Get pattern coordinates
    let pattern_from = vec4<f32>(in.pattern_from);
    let pattern_to = vec4<f32>(in.pattern_to);

    let pattern_tl_a = pattern_from.xy;
    let pattern_br_a = pattern_from.zw;
    let pattern_tl_b = pattern_to.xy;
    let pattern_br_b = pattern_to.zw;

    let pixelRatio = paintParams.pixel_ratio;
    let tileZoomRatio = drawable.tile_ratio;
    let fromScale = props.from_scale;
    let toScale = props.to_scale;

    let display_size_a = vec2<f32>(
        (pattern_br_a.x - pattern_tl_a.x) / pixelRatio,
        (pattern_br_a.y - pattern_tl_a.y) / pixelRatio
    );
    let display_size_b = vec2<f32>(
        (pattern_br_b.x - pattern_tl_b.x) / pixelRatio,
        (pattern_br_b.y - pattern_tl_b.y) / pixelRatio
    );

    // Determine position for pattern
    let pos = select(
        vec2<f32>(edgedistance, z * drawable.height_factor),
        vec2<f32>(f32(in.position.x), f32(in.position.y)),
        normal.x == 1.0 && normal.y == 0.0 && normal.z == 16384.0
    );

    // Calculate lighting
    var lighting = vec4<f32>(0.0, 0.0, 0.0, 1.0);
    var directional = clamp(dot(normal / 16383.0, props.light_position_base.xyz), 0.0, 1.0);
    directional = mix(1.0 - props.light_intensity, max(0.5 + props.light_intensity, 1.0), directional);

    if (normal.y != 0.0) {
        directional *= (
            (1.0 - props.vertical_gradient) +
            (props.vertical_gradient * clamp((t + base) * pow(height / 150.0, 0.5),
                mix(0.7, 0.98, 1.0 - props.light_intensity), 1.0))
        );
    }

    lighting = vec4<f32>(
        clamp(directional * props.light_color_pad.rgb,
            mix(vec3<f32>(0.0), vec3<f32>(0.3), 1.0 - props.light_color_pad.rgb),
            vec3<f32>(1.0)),
        1.0
    );
    lighting *= props.opacity;

    // Use get_pattern_pos helper for pattern positioning
    out.position = position;
    out.lighting = lighting;
    out.pos_a = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        fromScale * display_size_a,
        tileZoomRatio,
        pos
    );
    out.pos_b = get_pattern_pos(
        drawable.pixel_coord_upper,
        drawable.pixel_coord_lower,
        toScale * display_size_b,
        tileZoomRatio,
        pos
    );
    out.pattern_from = pattern_from;
    out.pattern_to = pattern_to;

    return out;
}
)";

    static constexpr auto fragment = R"(
struct FragmentInput {
    @location(0) lighting: vec4<f32>,
    @location(1) pos_a: vec2<f32>,
    @location(2) pos_b: vec2<f32>,
    @location(3) pattern_from: vec4<f32>,
    @location(4) pattern_to: vec4<f32>,
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

    let pattern_tl_a = in.pattern_from.xy;
    let pattern_br_a = in.pattern_from.zw;
    let pattern_tl_b = in.pattern_to.xy;
    let pattern_br_b = in.pattern_to.zw;

    // Sample pattern A
    let imagecoord_a = gl_mod(in.pos_a, vec2<f32>(1.0));
    let pos_a = mix(pattern_tl_a / tileProps.texsize, pattern_br_a / tileProps.texsize, imagecoord_a);
    let color_a = textureSample(pattern_texture, texture_sampler, pos_a);

    // Sample pattern B
    let imagecoord_b = gl_mod(in.pos_b, vec2<f32>(1.0));
    let pos_b = mix(pattern_tl_b / tileProps.texsize, pattern_br_b / tileProps.texsize, imagecoord_b);
    let color_b = textureSample(pattern_texture, texture_sampler, pos_b);

    // Mix patterns and apply lighting
    let pattern_color = mix(color_a, color_b, props.fade);
    return pattern_color * in.lighting;
}
)";
};

} // namespace shaders
} // namespace mbgl
