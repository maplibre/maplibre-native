#pragma once

#define MLN_MTL_UNIFORM_BLOCK(idx, vert, frag, struc) \
    { idx, vert, frag, sizeof(struc), MLN_STRINGIZE(struc) }

namespace mbgl {
namespace shaders {

constexpr auto prelude = R"(
#include <metal_stdlib>
using namespace metal;

enum class AttributeSource : int32_t {
    Constant,
    PerVertex,
    Computed,
};

struct Expression {};

struct Attribute {
    AttributeSource source;
    Expression expression;
};

struct alignas(16) ExpressionInputsUBO {
    float zoom;
    float time;
    uint64_t frame;
};

// Unpack a pair of values that have been packed into a single float.
// The packed values are assumed to be 8-bit unsigned integers, and are
// packed like so: packedValue = floor(input[0]) * 256 + input[1],
float2 unpack_float(const float packedValue) {
    const int packedIntValue = int(packedValue);
    const int v0 = packedIntValue / 256;
    return float2(v0, packedIntValue - v0 * 256);
}
float2 unpack_opacity(const float packedOpacity) {
    return float2(float(int(packedOpacity) / 2) / 127.0, fmod(packedOpacity, 2.0));
}
// To minimize the number of attributes needed, we encode a 4-component
// color into a pair of floats (i.e. a vec2) as follows:
// [ floor(color.r * 255) * 256 + color.g * 255, floor(color.b * 255) * 256 + color.g * 255 ]
float4 decode_color(const float2 encoded) {
    return float4(unpack_float(encoded[0]) / 255, unpack_float(encoded[1]) / 255);
}
// Unpack a pair of paint values and interpolate between them.
float unpack_mix_float(const float2 packedValue, const float t) {
    return mix(packedValue[0], packedValue[1], t);
}
// Unpack a pair of paint values and interpolate between them.
float4 unpack_mix_color(const float4 packedColors, const float t) {
    return mix(decode_color(float2(packedColors[0], packedColors[1])),
               decode_color(float2(packedColors[2], packedColors[3])), t);
}

float valueFor(device const Attribute& attrib,
               device const float& constValue,
               thread const float2& vertexValue,
               device const float& t,
               device const ExpressionInputsUBO&) {
    switch (attrib.source) {
        case AttributeSource::PerVertex: return unpack_mix_float(vertexValue, t);
        case AttributeSource::Computed:  // TODO
        default:
        case AttributeSource::Constant: return constValue;
    }
}
// single packed color
float4 colorFor(device const Attribute& attrib,
                device const float4& constValue,
                thread const float2& vertexValue,
                device const ExpressionInputsUBO&) {
    switch (attrib.source) {
        case AttributeSource::PerVertex: return decode_color(float2(vertexValue[0], vertexValue[1]));
        case AttributeSource::Computed:  // TODO
        default:
        case AttributeSource::Constant: return constValue;
    }
}
// interpolated packed colors
float4 colorFor(device const Attribute& attrib,
                device const float4& constValue,
                thread const float4& vertexValue,
                device const float& t,
                device const ExpressionInputsUBO&) {
    switch (attrib.source) {
        case AttributeSource::PerVertex: return unpack_mix_color(vertexValue, t);
        case AttributeSource::Computed:  // TODO
        default:
        case AttributeSource::Constant: return constValue;
    }
}

)";

}
} // namespace mbgl
