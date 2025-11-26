#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>
#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp> // Include the UBO definition

namespace mbgl {
namespace shaders {

constexpr auto hillshadePrepareShaderPrelude = R"(

#define idHillshadePrepareDrawableUBO       idDrawableReservedVertexOnlyUBO
#define idHillshadePrepareTilePropsUBO      drawableReservedUBOCount // Next available UBO index

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

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

// Matches HillshadePrepareDrawableUBO
struct HillshadePrepareDrawableUBO {
    mat4 matrix;
};

// Matches HillshadePrepareTilePropsUBO
struct HillshadePrepareTilePropsUBO {
    vec4 unpack;
    vec2 dimension;
    float zoom;
    float maxzoom;
};

layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = idHillshadePrepareDrawableUBO) readonly buffer HillshadePrepareDrawableUBOVector {
    HillshadePrepareDrawableUBO drawable_ubo[];
} drawableVector;

layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = idHillshadePrepareTilePropsUBO) readonly buffer HillshadePrepareTilePropsUBOVector {
    HillshadePrepareTilePropsUBO tileProps_ubo[];
} tilePropsVector;

layout(location = 0) out vec2 frag_position;

void main() {
    const HillshadePrepareDrawableUBO drawable = drawableVector.drawable_ubo[constant.ubo_index];
    const HillshadePrepareTilePropsUBO tileProps = tilePropsVector.tileProps_ubo[constant.ubo_index];

    gl_Position = drawable.matrix * vec4(in_position, 0, 1);

    vec2 epsilon = 1.0 / tileProps.dimension;
    float scale = (tileProps.dimension.x - 2.0) / tileProps.dimension.x;
    frag_position = (vec2(in_texture_position) / 8192.0) * scale + epsilon;
}
)";
    static constexpr auto fragment = R"(

layout(location = 0) in vec2 frag_position;

layout(set = TEXTURE_SET_INDEX, binding = 0) uniform sampler2D u_image;

layout(push_constant) uniform Constants {
    int ubo_index;
} constant;

// Matches HillshadePrepareTilePropsUBO
struct HillshadePrepareTilePropsUBO {
    vec4 unpack;
    vec2 dimension;
    float zoom;
    float maxzoom;
};

layout(std140, set = DRAWABLE_UBO_SET_INDEX, binding = idHillshadePrepareTilePropsUBO) readonly buffer HillshadePrepareTilePropsUBOVector {
    HillshadePrepareTilePropsUBO tileProps_ubo[];
} tilePropsVector;

float getElevation(vec2 coord, float bias) {
    const HillshadePrepareTilePropsUBO tileProps = tilePropsVector.tileProps_ubo[constant.ubo_index];
    // Convert encoded elevation value to meters
    vec4 data = texture(u_image, coord) * 255.0;
    data.a = -1.0;
    return dot(data, tileProps.unpack);
}

void main() {
    const HillshadePrepareTilePropsUBO tileProps = tilePropsVector.tileProps_ubo[constant.ubo_index];
    vec2 epsilon = 1.0 / tileProps.dimension;
    float tileSize = tileProps.dimension.x - 2.0;

    // queried pixels (using Sobel operator kernel):
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

    float a = getElevation(frag_position + vec2(-epsilon.x, -epsilon.y), 0.0);
    float b = getElevation(frag_position + vec2(0, -epsilon.y), 0.0);
    float c = getElevation(frag_position + vec2(epsilon.x, -epsilon.y), 0.0);
    float d = getElevation(frag_position + vec2(-epsilon.x, 0), 0.0);
  //float e = getElevation(frag_position, 0.0);
    float f = getElevation(frag_position + vec2(epsilon.x, 0), 0.0);
    float g = getElevation(frag_position + vec2(-epsilon.x, epsilon.y), 0.0);
    float h = getElevation(frag_position + vec2(0, epsilon.y), 0.0);
    float i = getElevation(frag_position + vec2(epsilon.x, epsilon.y), 0.0);

    // Convert the raw pixel-space derivative (slope) into world-space slope.
    // The conversion factor is: tileSize / (8 * meters_per_pixel).
    // meters_per_pixel is calculated as pow(2.0, 28.2562 - u_zoom).
    // The exaggeration factor is applied to scale the effect at lower zooms.
    float exaggeration = tileProps.zoom < 2.0 ? 0.4 : tileProps.zoom < 4.5 ? 0.35 : 0.3;

    vec2 deriv = vec2(
        (c + f + f + i) - (a + d + d + g),
        (g + h + h + i) - (a + b + b + c)
    ) * tileSize / pow(2.0, (tileProps.zoom - tileProps.maxzoom) * exaggeration + 28.2562 - tileProps.zoom); 

    // Encode the derivative into the color channels (r and g)
    // The derivative is scaled from world-space slope to the range [0, 1] for texture storage.
    fragColor = clamp(vec4(
        deriv.x / 8.0 + 0.5,
        deriv.y / 8.0 + 0.5,
        1.0,
        1.0), 0.0, 1.0);
#ifdef OVERDRAW_INSPECTOR
    fragColor = vec4(1.0);
#endif
}
)";
};

} // namespace shaders
} // namespace mbgl
