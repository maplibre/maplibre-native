#pragma once

namespace mbgl {
namespace shaders {

constexpr auto prelude = R"(
#include <metal_stdlib>
using namespace metal;

// The maximum allowed miter limit is 2.0 at the moment. the extrude normal is stored
// in a byte (-128..127). We scale regular normals up to length 63, but there are also
// "special" normals that have a bigger length (of up to 126 in this case).
#define LINE_NORMAL_SCALE (1.0 / (127 / 2))

// The attribute conveying progress along a line is scaled to [0, 2^15).
#define MAX_LINE_DISTANCE 32767.0

#define SDF_PX 8.0

// OpenGL `mod` is `x-y*floor(x/y)` where `floor` rounds down.
// Metal `fmod` is `x-y*trunc(x/y)` where `trunc` rounds toward zero.
// This function provides GL-compatible modulus for porting GLSL shaders.
template <typename T1, typename T2>
inline auto glMod(T1 x, T2 y) { return x - y * metal::floor(x/y); }

float radians(float degrees) {
    return M_PI_F * degrees / 180.0;
}

// Unpack a pair of values that have been packed into a single float.
// The packed values are assumed to be 8-bit unsigned integers, and are
// packed like so: packedValue = floor(input[0]) * 256 + input[1],
float2 unpack_float(const float packedValue) {
    const int packedIntValue = int(packedValue);
    const int v0 = packedIntValue / 256;
    return float2(v0, packedIntValue - v0 * 256);
}
float2 unpack_opacity(const float packedOpacity) {
    return float2(float(int(packedOpacity) / 2) / 127.0, glMod(packedOpacity, 2.0));
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



template<class ForwardIt, class T>
ForwardIt upper_bound(ForwardIt first, ForwardIt last, thread const T& value)
{
    size_t count = last - first;
    while (count > 0)
    {
        ForwardIt it = first;
        const size_t step = count / 2;
        it += step;
        if (!(value < *it))
        {
            first = ++it;
            count -= step + 1;
        }
        else {
            count = step;
        }
    }
    return first;
}

float interpolationFactor(float base, float rangeMin, float rangeMax, float z) {
    const float zoomDiff = rangeMax - rangeMin;
    const float zoomProgress = z - rangeMin;
    if (zoomDiff == 0) {
        return 0;
    } else if (base == 1.0f) {
        return zoomProgress / zoomDiff;
    } else {
        return (pow(base, zoomProgress) - 1) / (pow(base, zoomDiff) - 1);
    }
}


enum class GPUInterpType : uint16_t {
    Step,
    Linear,
    Exponential,
    Bezier
};
enum class GPUOutputType : uint16_t {
    Float,
    Color,
};
enum class GPUOptions : uint16_t {
    None = 0,
    IntegerZoom = 1 << 0,
    Transitioning = 1 << 1,
};
bool operator&(GPUOptions a, GPUOptions b) { return (uint16_t)a & (uint16_t)b; }

constant const int maxExprStops = 16;
struct alignas(16) GPUExpression {
    GPUOutputType outputType;
    uint16_t stopCount;
    GPUOptions options;
    GPUInterpType interpolation;
    
    union InterpOptions {
        struct Exponential {
            float base;
        } exponential;
        
        struct Bezier {
            float x1;
            float y1;
            float x2;
            float y2;
        } bezier;
    } interpOptions;
    
    float inputs[maxExprStops];
    
    union Stops {
        float floats[maxExprStops];
        float2 colors[maxExprStops];
    } stops;
    
    float eval(float zoom) device const {
        const auto effectiveZoom = (options & GPUOptions::IntegerZoom) ? floor(zoom) : zoom;
        const auto index = find(effectiveZoom);
        if (index == 0) {
            return stops.floats[0];
        } else if (index == stopCount) {
            return stops.floats[stopCount - 1];
        }
        switch (interpolation) {
            case GPUInterpType::Step: return stops.floats[index - 1];
            default: assert(false);
                [[fallthrough]];
            case GPUInterpType::Linear: assert(interpOptions.exponential.base == 1.0f);
                [[fallthrough]];
            case GPUInterpType::Exponential: {
                const float rangeBeg = inputs[index - 1];
                const float rangeEnd = inputs[index];
                const auto t = interpolationFactor(interpOptions.exponential.base, rangeBeg, rangeEnd, effectiveZoom);
                return mix(stops.floats[index - 1], stops.floats[index], t);
            }
            case GPUInterpType::Bezier:
                assert(false);
                return stops.floats[0];
        }
    }

    float4 evalColor(float zoom) device const {
        const auto effectiveZoom = (options & GPUOptions::IntegerZoom) ? floor(zoom) : zoom;
        const auto index = find(effectiveZoom);
        if (index == 0) {
            return getColor(0);
        } else if (index == stopCount) {
            return getColor(stopCount - 1);
        }
        switch (interpolation) {
            case GPUInterpType::Step:
                return getColor(index - 1);
            default:
                assert(false);
                [[fallthrough]];
            case GPUInterpType::Linear:
                assert(interpOptions.exponential.base == 1.0f);
                [[fallthrough]];
            case GPUInterpType::Exponential: {
                const float rangeBeg = inputs[index - 1];
                const float rangeEnd = inputs[index];
                const auto t = interpolationFactor(interpOptions.exponential.base, rangeBeg, rangeEnd, effectiveZoom);
                return mix(getColor(index - 1), getColor(index), clamp(t, 0.0, 1.0));
            }
            case GPUInterpType::Bezier:
                assert(false);
                return getColor(0);
        }
    }

    /// Get the index of the entry to use from the zoom level
    size_t find(float zoom) device const {
        return upper_bound(&inputs[0], &inputs[stopCount], zoom) - &inputs[0];
    }

    float4 getColor(size_t index) device const { return decode_color(stops.colors[index]); }
};
static_assert(sizeof(GPUExpression) == 32 + (4 + 8) * maxExprStops, "wrong alignment");
static_assert(sizeof(GPUExpression) % 16 == 0, "wrong alignment");


enum {
    idGlobalPaintParamsUBO,
    globalUBOCount
};

enum {
    idLineDrawableUBO = globalUBOCount,
    idLineInterpolationUBO,
    idLineTilePropertiesUBO,
    idLineEvaluatedPropsUBO,
    idLineExpressionUBO,
    lineUBOCount
};


enum class LineExpressionMask : uint32_t {
    None = 0,
    Color = 1 << 0,
    Opacity = 1 << 1,
    Blur = 1 << 2,
    Width = 1 << 3,
    GapWidth = 1 << 4,
    FloorWidth = 1 << 5,
    Offset = 1 << 6,
};
bool operator&(LineExpressionMask a, LineExpressionMask b) { return (uint32_t)a & (uint32_t)b; }

struct alignas(16) LineExpressionUBO {
    GPUExpression color;
    GPUExpression blur;
    GPUExpression opacity;
    GPUExpression gapwidth;
    GPUExpression offset;
    GPUExpression width;
    GPUExpression floorwidth;
};
static_assert(sizeof(LineExpressionUBO) % 16 == 0, "wrong alignment");

	
struct alignas(16) GlobalPaintParamsUBO {
    /*  0 */ float2 pattern_atlas_texsize;
    /*  8 */ float2 units_to_pixels;
    /* 16 */ float2 world_size;
    /* 24 */ float camera_to_center_distance;
    /* 28 */ float symbol_fade_change;
    /* 32 */ float aspect_ratio;
    /* 36 */ float pixel_ratio;
    /* 40 */ float zoom;
    /* 44 */ float pad1;
    /* 48 */
};
static_assert(sizeof(GlobalPaintParamsUBO) == 3 * 16, "unexpected padding");

struct alignas(16) FillEvaluatedPropsUBO {
    float4 color;
    float4 outline_color;
    float opacity;
    float fade;
    float from_scale;
    float to_scale;
};

struct alignas(16) FillExtrusionDrawableUBO {
    /*  0 */ float4x4 matrix;
    /* 64 */ float2 texsize;
    /* 72 */ float2 pixel_coord_upper;
    /* 80 */ float2 pixel_coord_lower;
    /* 88 */ float height_factor;
    /* 92 */ float tile_ratio;
    /* 96 */
};
static_assert(sizeof(FillExtrusionDrawableUBO) == 6 * 16, "unexpected padding");

struct alignas(16) FillExtrusionPropsUBO {
    /*  0 */ float4 color;
    /* 16 */ float4 light_color_pad;
    /* 32 */ float4 light_position_base;
    /* 48 */ float height;
    /* 52 */ float light_intensity;
    /* 56 */ float vertical_gradient;
    /* 60 */ float opacity;
    /* 64 */ float fade;
    /* 68 */ float from_scale;
    /* 72 */ float to_scale;
    /* 76 */ float pad2;
    /* 80 */
};
static_assert(sizeof(FillExtrusionPropsUBO) == 5 * 16, "unexpected padding");

struct alignas(16) FillExtrusionTilePropsUBO {
    /*  0 */ float4 pattern_from;
    /* 16 */ float4 pattern_to;
    /* 32 */
};
static_assert(sizeof(FillExtrusionTilePropsUBO) == 2 * 16, "unexpected padding");

struct alignas(16) FillExtrusionInterpolateUBO {
    /*  0 */ float base_t;
    /*  4 */ float height_t;
    /*  8 */ float color_t;
    /* 12 */ float pattern_from_t;
    /* 16 */ float pattern_to_t;
    /* 20 */ float pad1, pad2, pad3;
    /* 32 */
};
static_assert(sizeof(FillExtrusionInterpolateUBO) == 2 * 16, "unexpected padding");

struct alignas(16) LineEvaluatedPropsUBO {
    float4 color;
    float blur;
    float opacity;
    float gapwidth;
    float offset;
    float width;
    float floorwidth;
    LineExpressionMask expressionMask;
    float pad1;
};

struct alignas(16) SymbolDrawableUBO {
    float4x4 matrix;
    float4x4 label_plane_matrix;
    float4x4 coord_matrix;

    float2 texsize;
    float2 texsize_icon;

    float gamma_scale;
    /*bool*/ int rotate_symbol;
    float2 pad;
};
static_assert(sizeof(SymbolDrawableUBO) == 14 * 16, "unexpected padding");

struct alignas(16) SymbolTilePropsUBO {
    /*bool*/ int is_text;
    /*bool*/ int is_halo;
    /*bool*/ int pitch_with_map;
    /*bool*/ int is_size_zoom_constant;
    /*bool*/ int is_size_feature_constant;
    float size_t;
    float size;
    float padding;
};
static_assert(sizeof(SymbolTilePropsUBO) == 2 * 16, "unexpected padding");

struct alignas(16) SymbolInterpolateUBO {
    float fill_color_t;
    float halo_color_t;
    float opacity_t;
    float halo_width_t;
    float halo_blur_t;
    float pad1, pad2, pad3;
};
static_assert(sizeof(SymbolInterpolateUBO) == 32, "unexpected padding");

struct alignas(16) SymbolEvaluatedPropsUBO {
    float4 text_fill_color;
    float4 text_halo_color;
    float text_opacity;
    float text_halo_width;
    float text_halo_blur;
    float pad1;
    float4 icon_fill_color;
    float4 icon_halo_color;
    float icon_opacity;
    float icon_halo_width;
    float icon_halo_blur;
    float pad2;
};
static_assert(sizeof(SymbolEvaluatedPropsUBO) == 6 * 16, "unexpected padding");

// unpack pattern position
inline float2 get_pattern_pos(const float2 pixel_coord_upper, const float2 pixel_coord_lower,
                     const float2 pattern_size, const float tile_units_to_pixels, const float2 pos) {
    const float2 offset = glMod(glMod(glMod(pixel_coord_upper, pattern_size) * 256.0, pattern_size) * 256.0 + pixel_coord_lower, pattern_size);
    return (tile_units_to_pixels * pos + offset) / pattern_size;
}

)";

}
} // namespace mbgl
