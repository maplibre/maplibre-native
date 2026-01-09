#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>

namespace mbgl {
namespace shaders {

constexpr auto hillshadePrepareShaderPrelude = R"(

#define idHillshadePrepareDrawableUBO       idDrawableReservedVertexOnlyUBO
#define idHillshadePrepareTilePropsUBO      drawableReservedUBOCount

)";

template <>
struct ShaderSource<BuiltIn::HillshadePrepareShader, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "HillshadePrepareShader";

    static const std::array<AttributeInfo, 2> attributes;
    static constexpr std::array<AttributeInfo, 0> instanceAttributes{};
    static const std::array<TextureInfo, 1> textures;

    static constexpr auto prelude = hillshadePrepareShaderPrelude;
    static constexpr auto vertex = R"(

layout(location = 0) in ivec2 in_position;
layout(location = 1) in ivec2 in_texture_position;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idHillshadePrepareDrawableUBO) uniform HillshadePrepareDrawableUBO {
    mat4 matrix;
} drawable;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idHillshadePrepareTilePropsUBO) uniform HillshadePrepareTilePropsUBO {
    vec4 unpack;
    vec2 dimension;
    float zoom;
    float maxzoom;
} tileProps;

layout(location = 0) out vec2 frag_position;

void main() {

    gl_Position = drawable.matrix * vec4(in_position, 0.0, 1.0);
    applySurfaceTransform();

    const vec2 epsilon = vec2(1.0) / tileProps.dimension;
    const float scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    frag_position = in_texture_position / 8192.0 * scale + epsilon;
}
)";

    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position;
layout(location = 0) out vec4 out_color;

layout(set = DRAWABLE_UBO_SET_INDEX, binding = idHillshadePrepareTilePropsUBO) uniform HillshadePrepareTilePropsUBO {
    vec4 unpack;
    vec2 dimension;
    float zoom;
    float maxzoom;
} tileProps;

layout(set = DRAWABLE_IMAGE_SET_INDEX, binding = 0) uniform sampler2D image_sampler;

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

    const vec2 epsilon = 1.0 / tileProps.dimension;

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

    float a = getElevation(frag_position + vec2(-epsilon.x, -epsilon.y), 0.0, image_sampler, tileProps.unpack);
    float b = getElevation(frag_position + vec2(0, -epsilon.y), 0.0, image_sampler, tileProps.unpack);
    float c = getElevation(frag_position + vec2(epsilon.x, -epsilon.y), 0.0, image_sampler, tileProps.unpack);
    float d = getElevation(frag_position + vec2(-epsilon.x, 0), 0.0, image_sampler, tileProps.unpack);
  //float e = getElevation(frag_position, 0.0, image_sampler, tileProps.unpack);
    float f = getElevation(frag_position + vec2(epsilon.x, 0), 0.0, image_sampler, tileProps.unpack);
    float g = getElevation(frag_position + vec2(-epsilon.x, epsilon.y), 0.0, image_sampler, tileProps.unpack);
    float h = getElevation(frag_position + vec2(0, epsilon.y), 0.0, image_sampler, tileProps.unpack);
    float i = getElevation(frag_position + vec2(epsilon.x, epsilon.y), 0.0, image_sampler, tileProps.unpack);

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
    float exaggeration = tileProps.zoom < 2.0 ? 0.4 : tileProps.zoom < 4.5 ? 0.35 : 0.3;

    vec2 deriv = vec2(
        (c + f + f + i) - (a + d + d + g),
        (g + h + h + i) - (a + b + b + c)
    ) /  pow(2.0, (tileProps.zoom - tileProps.maxzoom) * exaggeration + 19.2562 - tileProps.zoom);

    out_color = clamp(vec4(
        deriv.x / 2.0 + 0.5,
        deriv.y / 2.0 + 0.5,
        1.0,
        1.0), 0.0, 1.0);
}
)";
};

} // namespace shaders
} // namespace mbgl
