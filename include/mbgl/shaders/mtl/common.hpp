#pragma once

#define MLN_MTL_UNIFORM_BLOCK(idx, vert, frag, struc) \
    { idx, vert, frag, sizeof(struc), MLN_STRINGIZE(struc) }

namespace mbgl {
namespace shaders {

constexpr auto prelude = R"(
#include <metal_stdlib>
using namespace metal;

// The maximum allowed miter limit is 2.0 at the moment. the extrude normal is stored
// in a byte (-128..127). We scale regular normals up to length 63, but there are also
// "special" normals that have a bigger length (of up to 126 in this case).
constant float LINE_NORMAL_SCALE = 1.0 / (127 / 2);

// The attribute conveying progress along a line is scaled to [0, 2^15).
constant float MAX_LINE_DISTANCE = 32767.0;

constant float SDF_PX = 8.0;

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


struct alignas(16) LineUBO {
    float4x4 matrix;
    float2 units_to_pixels;
    float ratio;
    float device_pixel_ratio;
};

struct alignas(16) LineGradientUBO {
    float4x4 matrix;
    float2 units_to_pixels;
    float ratio;
    float device_pixel_ratio;
};

struct alignas(16) LinePropertiesUBO {
    float4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float pad1, pad2, pad3;
};

struct alignas(16) LineGradientPropertiesUBO {
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float pad1, pad2, pad3;
};

struct alignas(16) LinePermutationUBO {
    Attribute color;
    Attribute blur;
    Attribute opacity;
    Attribute gapwidth;
    Attribute offset;
    Attribute width;
    Attribute floorwidth;
    Attribute pattern_from;
    Attribute pattern_to;
    bool overdrawInspector;
    uint8_t pad1, pad2, pad3;
    float pad4;
};

struct alignas(16) LineInterpolationUBO {
    float color_t;
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1, pad2;
};

struct alignas(16) LineGradientInterpolationUBO {
    float blur_t;
    float opacity_t;
    float gapwidth_t;
    float offset_t;
    float width_t;
    float pad1, pad2, pad3;
};


struct alignas(16) SymbolDrawableTilePropsUBO {
    /*bool*/ int is_text;
    /*bool*/ int is_halo;
    /*bool*/ int pitch_with_map;
    /*bool*/ int is_size_zoom_constant;
    /*bool*/ int is_size_feature_constant;
    float size_t;
    float size;
    float padding;
};

struct alignas(16) SymbolDrawableInterpolateUBO {
    float fill_color_t;
    float halo_color_t;
    float opacity_t;
    float halo_width_t;
    float halo_blur_t;
    float pad1, pad2, pad3;
};

struct alignas(16) SymbolDrawableUBO {
    float4 matrix;
    float4 label_plane_matrix;
    float4 coord_matrix;

    float2 texsize;
    float2 texsize_icon;

    float gamma_scale;
    float device_pixel_ratio;

    float camera_to_center_distance;
    float pitch;
    /*bool*/ int rotate_symbol;
    float aspect_ratio;
    float fade_change;
    float pad;
};

struct alignas(16) SymbolDrawablePaintUBO {
    float4 fill_color;
    float4 halo_color;
    float opacity;
    float halo_width;
    float halo_blur;
    float padding;
};

struct alignas(16) SymbolPermutationUBO {
    Attribute fill_color;
    Attribute halo_color;
    Attribute opacity;
    Attribute halo_width;
    Attribute halo_blur;
    Attribute padding;
    int32_t /*bool*/ overdrawInspector;
    float pad;
};

)";

}
} // namespace mbgl
