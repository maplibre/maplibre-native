#pragma once

#include <mbgl/shaders/shader_source.hpp>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::Prelude, gfx::Backend::Type::Vulkan> {
    static constexpr const char* name = "Prelude";

    static constexpr auto vertex = R"(

    // Unpack a pair of values that have been packed into a single float.
    // The packed values are assumed to be 8-bit unsigned integers, and are
    // packed like so:
    // packedValue = floor(input[0]) * 256 + input[1],
    vec2 unpack_float(const float packedValue) {
        int packedIntValue = int(packedValue);
        int v0 = packedIntValue / 256;
        return vec2(v0, packedIntValue - v0 * 256);
    }

    vec2 unpack_opacity(const float packedOpacity) {
        int intOpacity = int(packedOpacity) / 2;
        return vec2(float(intOpacity) / 127.0, mod(packedOpacity, 2.0));
    }

    // To minimize the number of attributes needed, we encode a 4-component
    // color into a pair of floats (i.e. a vec2) as follows:
    // [ floor(color.r * 255) * 256 + color.g * 255,
    //   floor(color.b * 255) * 256 + color.g * 255 ]
    vec4 decode_color(const vec2 encodedColor) {
        return vec4(
            unpack_float(encodedColor[0]) / 255.0,
            unpack_float(encodedColor[1]) / 255.0
        );
    }

    // Unpack a pair of paint values and interpolate between them.
    float unpack_mix_vec2(const vec2 packedValue, const float t) {
        return mix(packedValue[0], packedValue[1], t);
    }

    // Unpack a pair of paint values and interpolate between them.
    vec4 unpack_mix_color(const vec4 packedColors, const float t) {
        vec4 minColor = decode_color(vec2(packedColors[0], packedColors[1]));
        vec4 maxColor = decode_color(vec2(packedColors[2], packedColors[3]));
        return mix(minColor, maxColor, t);
    }

    )";

    static constexpr auto fragment = R"()";
};

} // namespace shaders
} // namespace mbgl
