// Solid Vulkan extrusion lighting. Paint values use the host's native binders;
// the extrusion color's alpha does not control opacity (style-spec semantics).
constexpr char vertexSource[] = R"glsl(
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_COLOR_IS_UNIFORM
layout(location = 2) in vec4 a_color_min;
layout(location = 3) in vec4 a_color_max;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_BASE_IS_UNIFORM
layout(location = 4) in vec2 a_base;
#endif
#if !MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_HEIGHT_IS_UNIFORM
layout(location = 5) in vec2 a_height;
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
} u;
layout(location = 0) out mediump vec4 frag_color;
void main() {
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_COLOR_IS_UNIFORM
    vec4 color = u.color;
#else
    vec4 color = mix(a_color_min, a_color_max, u.interpolation.x);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_BASE_IS_UNIFORM
    float base = u.base;
#else
    float base = max(mix(a_base.x, a_base.y, u.interpolation.y), 0.0);
#endif
#if MLN_PLUGIN_PROPERTY_FILL_EXTRUSION_HEIGHT_IS_UNIFORM
    float height = u.height;
#else
    float height = max(mix(a_height.x, a_height.y, u.interpolation.z), 0.0);
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
    float t = a_pos.z;
    float z = t > 0.0 ? height : base;
    gl_Position = u.matrix * vec4(a_pos.xy, z, 1.0);
    applySurfaceTransform();
    float luminance = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
    color += vec4(0.03, 0.03, 0.03, 1.0);
    float fraction = clamp(dot(a_normal, u.light_direction), 0.0, 1.0);
    float directional = mix(1.0 - u.intensity, max(1.0 - luminance + u.intensity, 1.0), fraction);
    if (a_normal.z == 0.0) {
        float factor = clamp((t + base) * pow(height / 150.0, 0.5), mix(0.7, 0.98, 1.0 - u.intensity), 1.0);
        directional *= (1.0 - gradient) + gradient * factor;
    }
    vec3 minLight = mix(vec3(0.0), vec3(0.3), 1.0 - u.light_color);
    frag_color = vec4(clamp(color.rgb * directional * u.light_color, minLight, vec3(1.0)), 1.0) * opacity;
}
)glsl";
constexpr char fragmentSource[] = R"glsl(
layout(location = 0) in vec4 frag_color;
layout(location = 0) out vec4 out_color;
void main() { out_color = frag_color; }
)glsl";
