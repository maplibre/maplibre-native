// Solid Vulkan extrusion lighting. Paint values use the host's native binders;
// the extrusion color's alpha does not control opacity (style-spec semantics).
constexpr char vertexSource[] = R"glsl(
layout(location = 0) in ivec2 a_pos;
#if MLN_PLUGIN_INSTANCED
layout(location = 1) in ivec2 a_next_pos;
layout(location = 13) in uvec2 a_next_decimals_edge;
#endif
layout(location = 8) in uvec2 a_decimals_edge;
#if MLN_PLUGIN_HAS_PATTERN && !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PATTERN_IS_UNIFORM
layout(location = 9) in vec4 a_pattern_from_min;
layout(location = 10) in vec4 a_pattern_from_max;
layout(location = 11) in vec4 a_pattern_to_min;
layout(location = 12) in vec4 a_pattern_to_max;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_COLOR_IS_UNIFORM
layout(location = 2) in vec4 a_color_min;
layout(location = 3) in vec4 a_color_max;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_BASE_IS_UNIFORM
layout(location = 4) in float a_base_min;
layout(location = 14) in float a_base_max;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_HEIGHT_IS_UNIFORM
layout(location = 5) in float a_height_min;
layout(location = 15) in float a_height_max;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_OPACITY_IS_UNIFORM
layout(location = 6) in vec2 a_opacity;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_VERTICAL_GRADIENT_IS_UNIFORM
layout(location = 7) in vec2 a_gradient;
#endif
layout(set = DRAWABLE_UBO_SET_INDEX, binding = MLN_PLUGIN_UNIFORM_0_BINDING) uniform ExtrusionUniforms {
    mat4 matrix;
    vec4 color;
    vec3 light_color; float intensity;
    vec3 light_direction; float base;
    float height; float opacity; float gradient; float pad;
    vec4 interpolation;
    vec4 interpolation2;
    vec4 pattern_from; vec4 pattern_to;
    vec2 pixel_upper; vec2 pixel_lower;
    float tile_ratio; float height_factor; float pixel_ratio; float from_scale;
    float to_scale; float fade; vec2 texture_size;
} u;
layout(location = 0) out mediump vec4 frag_color;
#if MLN_PLUGIN_HAS_PATTERN
layout(location = 1) out vec2 frag_pos_a;
layout(location = 2) out vec2 frag_pos_b;
layout(location = 3) out vec4 frag_pattern_from;
layout(location = 4) out vec4 frag_pattern_to;
#endif
void main() {
    uint packed = a_decimals_edge.x;
    vec2 p1 = vec2(a_pos) + vec2(packed >> 9, (packed >> 1) & 127u) / 128.0;
#if MLN_PLUGIN_INSTANCED
    if ((packed & 1u) != 0u) {
        gl_Position = vec4(0.0);
        frag_color = vec4(0.0);
        return;
    }
    uint next_packed = a_next_decimals_edge.x;
    vec2 p2 = vec2(a_next_pos) + vec2(next_packed >> 9, (next_packed >> 1) & 127u) / 128.0;
    vec2 direction = normalize(p2 - p1);
    vec3 normal = vec3(-direction.y, direction.x, 0.0);
    float t = float(gl_VertexIndex & 1);
    bool at_next = gl_VertexIndex < 2;
    vec2 pos = at_next ? p2 : p1;
    float edge = float(at_next ? a_decimals_edge.y : a_next_decimals_edge.y);
#else
    vec2 pos = p1;
    vec3 normal = vec3(0.0, 0.0, 1.0);
    float t = 1.0;
    float edge = 0.0;
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_COLOR_IS_UNIFORM
    vec4 color = u.color;
#else
    vec4 color = mix(a_color_min, a_color_max, u.interpolation.x);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_BASE_IS_UNIFORM
    float base = u.base;
#else
    float base = max(mix(a_base_min, a_base_max, u.interpolation.y), 0.0);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_HEIGHT_IS_UNIFORM
    float height = u.height;
#else
    float height = max(mix(a_height_min, a_height_max, u.interpolation.z), 0.0);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_OPACITY_IS_UNIFORM
    float opacity = u.opacity;
#else
    float opacity = mix(a_opacity.x, a_opacity.y, u.interpolation.w);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_VERTICAL_GRADIENT_IS_UNIFORM
    float gradient = u.gradient;
#else
    float gradient = u.interpolation2.x < 1.0 ? a_gradient.x : a_gradient.y;
#endif
    float z = t > 0.0 ? height : base;
    gl_Position = u.matrix * vec4(pos, z, 1.0);
    applySurfaceTransform();
#if !MLN_PLUGIN_COLOR_WRITE
    frag_color = vec4(0.0);
    return;
#endif
#if MLN_PLUGIN_HAS_PATTERN
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PATTERN_IS_UNIFORM
    vec4 pattern_from = u.pattern_from, pattern_to = u.pattern_to;
#else
    vec4 pattern_from = u.from_scale > 1.0 ? a_pattern_from_min : a_pattern_from_max;
    vec4 pattern_to = a_pattern_to_min;
#endif
    frag_pattern_from = pattern_from; frag_pattern_to = pattern_to;
    vec2 size_a = (pattern_from.zw-pattern_from.xy)/u.pixel_ratio;
    vec2 size_b = (pattern_to.zw-pattern_to.xy)/u.pixel_ratio;
    vec2 pattern_pos = normal.z == 0.0 ? vec2(edge,z*u.height_factor) : vec2(a_pos);
    frag_pos_a = get_pattern_pos(u.pixel_upper,u.pixel_lower,u.from_scale*size_a,u.tile_ratio,pattern_pos);
    frag_pos_b = get_pattern_pos(u.pixel_upper,u.pixel_lower,u.to_scale*size_b,u.tile_ratio,pattern_pos);
    float directional = clamp(dot(normal,u.light_direction),0.0,1.0);
    directional = mix(1.0-u.intensity,max(0.5+u.intensity,1.0),directional);
    if (normal.z == 0.0) {
        float factor = clamp((t+base)*pow(height/150.0,0.5),mix(0.7,0.98,1.0-u.intensity),1.0);
        directional *= (1.0-gradient)+gradient*factor;
    }
    frag_color = vec4(clamp(directional*u.light_color,mix(vec3(0.0),vec3(0.3),1.0-u.light_color),vec3(1.0)),1.0)*opacity;
#else
    float luminance = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
    color += vec4(0.03, 0.03, 0.03, 1.0);
    float fraction = clamp(dot(normal, u.light_direction), 0.0, 1.0);
    float directional = mix(1.0 - u.intensity, max(1.0 - luminance + u.intensity, 1.0), fraction);
    if (normal.z == 0.0) {
        float factor = clamp((t + base) * pow(height / 150.0, 0.5), mix(0.7, 0.98, 1.0 - u.intensity), 1.0);
        directional *= (1.0 - gradient) + gradient * factor;
    }
    vec3 minLight = mix(vec3(0.0), vec3(0.3), 1.0 - u.light_color);
    frag_color = vec4(clamp(color.rgb * directional * u.light_color, minLight, vec3(1.0)), 1.0) * opacity;
#endif
}
)glsl";
constexpr char fragmentSource[] = R"glsl(
layout(location = 0) in vec4 frag_color;
layout(location = 0) out vec4 out_color;
#if MLN_PLUGIN_HAS_PATTERN
layout(location = 1) in vec2 frag_pos_a;
layout(location = 2) in vec2 frag_pos_b;
layout(location = 3) in vec4 frag_pattern_from;
layout(location = 4) in vec4 frag_pattern_to;
layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image0_sampler;
layout(set = DRAWABLE_UBO_SET_INDEX, binding = MLN_PLUGIN_UNIFORM_0_BINDING) uniform ExtrusionUniforms {
    mat4 matrix;
    vec4 color;
    vec3 light_color; float intensity;
    vec3 light_direction; float base;
    float height; float opacity; float gradient; float pad;
    vec4 interpolation;
    vec4 interpolation2;
    vec4 pattern_from; vec4 pattern_to;
    vec2 pixel_upper; vec2 pixel_lower;
    float tile_ratio; float height_factor; float pixel_ratio; float from_scale;
    float to_scale; float fade; vec2 texture_size;
} u;
#endif
void main() {
#if !MLN_PLUGIN_COLOR_WRITE
    out_color = vec4(0.0);
#elif MLN_PLUGIN_HAS_PATTERN
    vec2 pos_a = mix(frag_pattern_from.xy/u.texture_size,frag_pattern_from.zw/u.texture_size,mod(frag_pos_a,1.0));
    vec2 pos_b = mix(frag_pattern_to.xy/u.texture_size,frag_pattern_to.zw/u.texture_size,mod(frag_pos_b,1.0));
    out_color = mix(texture(image0_sampler,pos_a),texture(image0_sampler,pos_b),u.fade)*frag_color;
#else
    out_color = frag_color;
#endif
}
)glsl";

// Metal translation of the shaders above. Metal has no separate fragment
// source: one file provides both entry points, named by vertex_entry_point
// and fragment_entry_point. Buffer/attribute bindings come from the host
// (see plugin_shader.cpp's Metal branch), so this only needs [[attribute(N)]]
// indices matching the plugin's declared attribute locations and the
// MLN_PLUGIN_UNIFORM_0_BINDING macro for the uniform buffer slot.
constexpr char metalSource[] = R"msl(
// attribute() indices below match the GLSL layout(location=N) values above —
// both tables list all 16 attributes now that patterns are supported, so
// locations are contiguous 0-15 without any Metal-specific renumbering.
struct VertexIn {
    short2 a_pos [[attribute(0)]];
#if MLN_PLUGIN_INSTANCED
    short2 a_next_pos [[attribute(1)]];
    ushort2 a_next_decimals_edge [[attribute(13)]];
#endif
    ushort2 a_decimals_edge [[attribute(8)]];
#if MLN_PLUGIN_HAS_PATTERN && !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PATTERN_IS_UNIFORM
    float4 a_pattern_from_min [[attribute(9)]];
    float4 a_pattern_from_max [[attribute(10)]];
    float4 a_pattern_to_min [[attribute(11)]];
    float4 a_pattern_to_max [[attribute(12)]];
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_COLOR_IS_UNIFORM
    float4 a_color_min [[attribute(2)]];
    float4 a_color_max [[attribute(3)]];
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_BASE_IS_UNIFORM
    float a_base_min [[attribute(4)]];
    float a_base_max [[attribute(14)]];
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_HEIGHT_IS_UNIFORM
    float a_height_min [[attribute(5)]];
    float a_height_max [[attribute(15)]];
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_OPACITY_IS_UNIFORM
    float2 a_opacity [[attribute(6)]];
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_VERTICAL_GRADIENT_IS_UNIFORM
    float2 a_gradient [[attribute(7)]];
#endif
};

// Byte layout must match the host's Uniforms struct (fill_extrusion.cpp)
// exactly: packed_float3 (not float3) keeps the trailing scalar tightly
// packed the way GLSL std140 and the plain C++ struct both do.
struct ExtrusionUniforms {
    float4x4 matrix;
    float4 color;
    packed_float3 light_color; float intensity;
    packed_float3 light_direction; float base;
    float height; float opacity; float gradient; float pad;
    float4 interpolation;
    float4 interpolation2;
    float4 pattern_from; float4 pattern_to;
    float2 pixel_upper; float2 pixel_lower;
    float tile_ratio; float height_factor; float pixel_ratio; float from_scale;
    float to_scale; float fade; float2 texture_size;
};

struct FragmentIn {
    float4 position [[position]];
    float4 frag_color;
#if MLN_PLUGIN_HAS_PATTERN
    float2 frag_pos_a;
    float2 frag_pos_b;
    float4 frag_pattern_from;
    float4 frag_pattern_to;
#endif
};

vertex FragmentIn vertexMain(VertexIn in [[stage_in]],
                             constant ExtrusionUniforms& u [[buffer(MLN_PLUGIN_UNIFORM_0_BINDING)]]
#if MLN_PLUGIN_INSTANCED
                             , uint vertexIndex [[vertex_id]]
#endif
                             ) {
    FragmentIn out;
    uint packed = uint(in.a_decimals_edge.x);
    float2 p1 = float2(in.a_pos) + float2(packed >> 9, (packed >> 1) & 127u) / 128.0;
#if MLN_PLUGIN_INSTANCED
    if ((packed & 1u) != 0u) {
        out.position = float4(0.0);
        out.frag_color = float4(0.0);
        return out;
    }
    uint next_packed = uint(in.a_next_decimals_edge.x);
    float2 p2 = float2(in.a_next_pos) + float2(next_packed >> 9, (next_packed >> 1) & 127u) / 128.0;
    float2 direction = normalize(p2 - p1);
    float3 normal = float3(-direction.y, direction.x, 0.0);
    float t = float(vertexIndex & 1u);
    bool at_next = vertexIndex < 2u;
    float2 pos = at_next ? p2 : p1;
    float edge = float(at_next ? in.a_decimals_edge.y : in.a_next_decimals_edge.y);
#else
    float2 pos = p1;
    float3 normal = float3(0.0, 0.0, 1.0);
    float t = 1.0;
    float edge = 0.0;
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_COLOR_IS_UNIFORM
    float4 color = u.color;
#else
    float4 color = mix(in.a_color_min, in.a_color_max, u.interpolation.x);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_BASE_IS_UNIFORM
    float base = u.base;
#else
    float base = max(mix(in.a_base_min, in.a_base_max, u.interpolation.y), 0.0);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_HEIGHT_IS_UNIFORM
    float height = u.height;
#else
    float height = max(mix(in.a_height_min, in.a_height_max, u.interpolation.z), 0.0);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_OPACITY_IS_UNIFORM
    float opacity = u.opacity;
#else
    float opacity = mix(in.a_opacity.x, in.a_opacity.y, u.interpolation.w);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_VERTICAL_GRADIENT_IS_UNIFORM
    float gradient = u.gradient;
#else
    float gradient = u.interpolation2.x < 1.0 ? in.a_gradient.x : in.a_gradient.y;
#endif
    float z = t > 0.0 ? height : base;
    out.position = u.matrix * float4(pos, z, 1.0);
    // Deliberately no early return for !MLN_PLUGIN_COLOR_WRITE here (unlike the
    // fragment function, which does discard/zero): keeping this function's
    // instructions identical between the depth-only and color-pass shader
    // variants avoids the compiler computing `out.position` with different
    // scheduling/rounding between the two, which showed up as Z-fighting
    // banding between the depth prepass and the translucent color pass.
#if MLN_PLUGIN_HAS_PATTERN
    float4 pattern_from;
    float4 pattern_to;
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_PATTERN_IS_UNIFORM
    pattern_from = u.pattern_from;
    pattern_to = u.pattern_to;
#else
    pattern_from = u.from_scale > 1.0 ? in.a_pattern_from_min : in.a_pattern_from_max;
    pattern_to = in.a_pattern_to_min;
#endif
    out.frag_pattern_from = pattern_from;
    out.frag_pattern_to = pattern_to;
    float2 size_a = (pattern_from.zw - pattern_from.xy) / u.pixel_ratio;
    float2 size_b = (pattern_to.zw - pattern_to.xy) / u.pixel_ratio;
    float2 pattern_pos = normal.z == 0.0 ? float2(edge, z * u.height_factor) : float2(in.a_pos);
    out.frag_pos_a = get_pattern_pos(u.pixel_upper, u.pixel_lower, u.from_scale * size_a, u.tile_ratio, pattern_pos);
    out.frag_pos_b = get_pattern_pos(u.pixel_upper, u.pixel_lower, u.to_scale * size_b, u.tile_ratio, pattern_pos);
    float directional = clamp(dot(normal, float3(u.light_direction)), 0.0, 1.0);
    directional = mix(1.0 - u.intensity, max(0.5 + u.intensity, 1.0), directional);
    if (normal.z == 0.0) {
        float factor = clamp((t + base) * pow(height / 150.0, 0.5), mix(0.7, 0.98, 1.0 - u.intensity), 1.0);
        directional *= (1.0 - gradient) + gradient * factor;
    }
    out.frag_color = float4(clamp(directional * float3(u.light_color),
                                  mix(float3(0.0), float3(0.3), 1.0 - float3(u.light_color)),
                                  float3(1.0)),
                            1.0) *
                     opacity;
#else
    float luminance = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
    color += float4(0.03, 0.03, 0.03, 1.0);
    float fraction = clamp(dot(normal, float3(u.light_direction)), 0.0, 1.0);
    float directional = mix(1.0 - u.intensity, max(1.0 - luminance + u.intensity, 1.0), fraction);
    if (normal.z == 0.0) {
        float factor = clamp((t + base) * pow(height / 150.0, 0.5), mix(0.7, 0.98, 1.0 - u.intensity), 1.0);
        directional *= (1.0 - gradient) + gradient * factor;
    }
    float3 minLight = mix(float3(0.0), float3(0.3), 1.0 - float3(u.light_color));
    out.frag_color = float4(clamp(color.rgb * directional * float3(u.light_color), minLight, float3(1.0)), 1.0) *
                     opacity;
#endif
    return out;
}

fragment float4 fragmentMain(FragmentIn in [[stage_in]]
#if MLN_PLUGIN_HAS_PATTERN
                             , constant ExtrusionUniforms& u [[buffer(MLN_PLUGIN_UNIFORM_0_BINDING)]]
                             , texture2d<float> image0 [[texture(0)]]
                             , sampler image0_sampler [[sampler(0)]]
#endif
                             ) {
#if !MLN_PLUGIN_COLOR_WRITE
    return float4(0.0);
#elif MLN_PLUGIN_HAS_PATTERN
    float2 pos_a = mix(in.frag_pattern_from.xy / u.texture_size,
                       in.frag_pattern_from.zw / u.texture_size,
                       glMod(in.frag_pos_a, 1.0));
    float2 pos_b = mix(in.frag_pattern_to.xy / u.texture_size,
                       in.frag_pattern_to.zw / u.texture_size,
                       glMod(in.frag_pos_b, 1.0));
    return mix(image0.sample(image0_sampler, pos_a), image0.sample(image0_sampler, pos_b), u.fade) * in.frag_color;
#else
    return in.frag_color;
#endif
}
)msl";
