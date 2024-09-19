#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "HillshadePrepareShader";

    static const std::array<UniformBlockInfo, 1> uniforms;
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec2 in_texture_position;

layout(set = 0, binding = 1) uniform HillshadePrepareDrawableUBO {
    mat4 matrix;
    vec4 unpack;
    vec2 dimension;
    float zoom;
    float maxzoom;
} drawable;

layout(location = 0) out vec2 frag_position;

void main() {

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    gl_Position.y *= -1.0;

    const vec2 epsilon = vec2(1.0) / drawable.dimension;
    const float scale = (drawable.dimension.x - 2.0) / drawable.dimension.x;
    frag_position = in_texture_position / 8192.0 * scale + epsilon;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position;
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform HillshadePrepareDrawableUBO {
    mat4 matrix;
    vec4 unpack;
    vec2 dimension;
    float zoom;
    float maxzoom;
} drawable;

layout(set = 1, binding = 0) uniform sampler2D image_sampler;

float getElevation(vec2 coord, float bias, sampler2D image_sampler, vec4 unpack) {
    // Convert encoded elevation value to meters
    vec4 data = texture(image_sampler, coord) * 255.0;
    data.a = -1.0;
    return dot(data, unpack) / 4.0;
}

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    const vec2 epsilon = 1.0 / drawable.dimension;

    // queried pixels:
    // +-----------+
    // |   |   |   |
    // | a | b | c |
    // |   |   |   |
    // +-----------+
    // |   |   |   |
    // | d | e | f |
    // |   |   |   |
    // +-----------+
    // |   |   |   |
    // | g | h | i |
    // |   |   |   |
    // +-----------+

    float a = getElevation(frag_position + vec2(-epsilon.x, -epsilon.y), 0.0, image_sampler, drawable.unpack);
    float b = getElevation(frag_position + vec2(0, -epsilon.y), 0.0, image_sampler, drawable.unpack);
    float c = getElevation(frag_position + vec2(epsilon.x, -epsilon.y), 0.0, image_sampler, drawable.unpack);
    float d = getElevation(frag_position + vec2(-epsilon.x, 0), 0.0, image_sampler, drawable.unpack);
  //float e = getElevation(frag_position, 0.0, image_sampler, drawable.unpack);
    float f = getElevation(frag_position + vec2(epsilon.x, 0), 0.0, image_sampler, drawable.unpack);
    float g = getElevation(frag_position + vec2(-epsilon.x, epsilon.y), 0.0, image_sampler, drawable.unpack);
    float h = getElevation(frag_position + vec2(0, epsilon.y), 0.0, image_sampler, drawable.unpack);
    float i = getElevation(frag_position + vec2(epsilon.x, epsilon.y), 0.0, image_sampler, drawable.unpack);

    // here we divide the x and y slopes by 8 * pixel size
    // where pixel size (aka meters/pixel) is:
    // circumference of the world / (pixels per tile * number of tiles)
    // which is equivalent to: 8 * 40075016.6855785 / (512 * pow(2, u_zoom))
    // which can be reduced to: pow(2, 19.25619978527 - u_zoom)
    // we want to vertically exaggerate the hillshading though, because otherwise
    // it is barely noticeable at low zooms. to do this, we multiply this by some
    // scale factor pow(2, (u_zoom - u_maxzoom) * a) where a is an arbitrary value
    // Here we use a=0.3 which works out to the expression below. see
    // nickidlugash's awesome breakdown for more info
    // https://github.com/mapbox/mapbox-gl-js/pull/5286#discussion_r148419556
    float exaggeration = drawable.zoom < 2.0 ? 0.4 : drawable.zoom < 4.5 ? 0.35 : 0.3;

    vec2 deriv = vec2(
        (c + f + f + i) - (a + d + d + g),
        (g + h + h + i) - (a + b + b + c)
    ) /  pow(2.0, (drawable.zoom - drawable.maxzoom) * exaggeration + 19.2562 - drawable.zoom);

    out_color = clamp(vec4(
        deriv.x / 2.0 + 0.5,
        deriv.y / 2.0 + 0.5,
        1.0,
        1.0), 0.0, 1.0);
}
)";
};

template <>
struct ShaderSource<BuiltIn::HillshadeShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "HillshadeShader";

    static const std::array<UniformBlockInfo, 2> uniforms;
    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec2 in_texture_position;

layout(set = 0, binding = 1) uniform HillshadeDrawableUBO {
    mat4 matrix;
    vec2 latrange;
    vec2 light;
} drawable;

layout(location = 0) out vec2 frag_position;

void main() {

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    gl_Position.y *= -1.0;

    frag_position = vec2(in_texture_position) / 8192.0;
    frag_position.y = 1.0 - frag_position.y; // TODO check this. prepare should ignore the flip
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position;
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform HillshadeDrawableUBO {
    mat4 matrix;
    vec2 latrange;
    vec2 light;
} drawable;

layout(set = 0, binding = 2) uniform HillshadeEvaluatedPropsUBO {
    vec4 highlight;
    vec4 shadow;
    vec4 accent;
} props;

layout(set = 1, binding = 0) uniform sampler2D image_sampler;

void main() {

#if defined(OVERDRAW_INSPECTOR)
    out_color = vec4(1.0);
    return;
#endif

    vec4 pixel = texture(image_sampler, frag_position);

    vec2 deriv = pixel.rg * 2.0 - 1.0;

    // We divide the slope by a scale factor based on the cosin of the pixel's approximate latitude
    // to account for mercator projection distortion. see #4807 for details
    float scaleFactor = cos(radians((drawable.latrange[0] - drawable.latrange[1]) * frag_position.y + drawable.latrange[1]));
    // We also multiply the slope by an arbitrary z-factor of 1.25
    float slope = atan(1.25 * length(deriv) / scaleFactor);
    float aspect = deriv.x != 0.0 ? atan(deriv.y, -deriv.x) : M_PI / 2.0 * (deriv.y > 0.0 ? 1.0 : -1.0);

    float intensity = drawable.light.x;
    // We add PI to make this property match the global light object, which adds PI/2 to the light's azimuthal
    // position property to account for 0deg corresponding to north/the top of the viewport in the style spec
    // and the original shader was written to accept (-illuminationDirection - 90) as the azimuthal.
    float azimuth = drawable.light.y + M_PI;

    // We scale the slope exponentially based on intensity, using a calculation similar to
    // the exponential interpolation function in the style spec:
    // https://github.com/mapbox/mapbox-gl-js/blob/master/src/style-spec/expression/definitions/interpolate.js#L217-L228
    // so that higher intensity values create more opaque hillshading.
    float base = 1.875 - intensity * 1.75;
    float maxValue = 0.5 * M_PI;
    float scaledSlope = intensity != 0.5 ? ((pow(base, slope) - 1.0) / (pow(base, maxValue) - 1.0)) * maxValue : slope;

    // The accent color is calculated with the cosine of the slope while the shade color is calculated with the sine
    // so that the accent color's rate of change eases in while the shade color's eases out.
    float accent = cos(scaledSlope);
    // We multiply both the accent and shade color by a clamped intensity value
    // so that intensities >= 0.5 do not additionally affect the color values
    // while intensity values < 0.5 make the overall color more transparent.
    vec4 accent_color = (1.0 - accent) * props.accent * clamp(intensity * 2.0, 0.0, 1.0);
    float shade = abs(mod((aspect + azimuth) / M_PI + 0.5, 2.0) - 1.0);
    vec4 shade_color = mix(props.shadow, props.highlight, shade) * sin(scaledSlope) * clamp(intensity * 2.0, 0.0, 1.0);
    vec4 color = accent_color * (1.0 - shade_color.a) + shade_color;

    out_color = color;
}
)";
};

} // namespace shaders
} // namespace mbgl
