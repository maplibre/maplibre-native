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

inline float radians(float degrees) {
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

// Unpack pattern position
inline float2 get_pattern_pos(const float2 pixel_coord_upper, const float2 pixel_coord_lower,
                     const float2 pattern_size, const float tile_units_to_pixels, const float2 pos) {
    const float2 offset = glMod(glMod(glMod(pixel_coord_upper, pattern_size) * 256.0, pattern_size) * 256.0 + pixel_coord_lower, pattern_size);
    return (tile_units_to_pixels * pos + offset) / pattern_size;
}

// Sample the terrain elevation in meters at a tile-local coordinate, with manual
// bilinear interpolation on DEM pixel centers (the DEM has a 1px backfilled border),
// matching the maplibre-gl-js get_elevation() prelude function. Unlike gl-js (global
// terrain uniforms), MapLibre Native carries the DEM data per-drawable, so the DEM
// texture and dem_* values are passed in as arguments rather than read from globals.
inline float get_elevation(float2 pos,
                           texture2d<float, access::sample> dem,
                           sampler dem_sampler,
                           float4 dem_coords,
                           float4 dem_unpack,
                           float dem_dim,
                           float dem_exaggeration,
                           float dem_enabled) {
    if (dem_enabled == 0.0) {
        return 0.0;
    }
    const float2 coord = (pos * dem_coords.x + dem_coords.yz) * dem_dim + 1.0;
    const float2 f = fract(coord);
    const float2 c = (floor(coord) + 0.5) / (dem_dim + 2.0);
    const float d = 1.0 / (dem_dim + 2.0);
    float4 tl = dem.sample(dem_sampler, c) * 255.0;
    tl.a = -1.0;
    float4 tr = dem.sample(dem_sampler, c + float2(d, 0.0)) * 255.0;
    tr.a = -1.0;
    float4 bl = dem.sample(dem_sampler, c + float2(0.0, d)) * 255.0;
    bl.a = -1.0;
    float4 br = dem.sample(dem_sampler, c + float2(d, d)) * 255.0;
    br.a = -1.0;
    const float elevation = mix(mix(dot(tl, dem_unpack), dot(tr, dem_unpack), f.x),
                                mix(dot(bl, dem_unpack), dot(br, dem_unpack), f.x),
                                f.y);
    return elevation * dem_exaggeration;
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
static_assert(sizeof(GPUExpression) == 32 + (4 + 8) * maxExprStops, "wrong size");
static_assert(sizeof(GPUExpression) % 16 == 0, "wrong alignment");

struct alignas(16) GlobalPaintParamsUBO {
    /*  0 */ float2 pattern_atlas_texsize;
    /*  8 */ float2 units_to_pixels;
    /* 16 */ float2 world_size;
    /* 24 */ float camera_to_center_distance;
    /* 28 */ float symbol_fade_change;
    /* 32 */ float aspect_ratio;
    /* 36 */ float pixel_ratio;
    /* 40 */ float map_zoom;
    /* 44 */ float pad1;
    /* 48 */ float4 drape_tile;
    /* 64 */
};
static_assert(sizeof(GlobalPaintParamsUBO) == 4 * 16, "wrong size");

// Place a clip-space position computed with a tile-local drape matrix into the
// current terrain drape render target (see the GL prelude for the derivation).
// `matrix` carries the drawable's tile (z, x, y) in its unused third column and
// target_tile is GlobalPaintParamsUBO::drape_tile (w != 0 while drawing into a
// drape target).
inline float4 apply_drape_transform(float4 clip, float4x4 matrix, float4 target_tile) {
    if (target_tile.w == 0.0) {
        return clip;
    }
    const float3 tile = float3(matrix[2][0], matrix[2][1], matrix[2][2]);
    const float k = target_tile.x - tile.x; // target zoom - drawable zoom
    const float scale = exp2(k);
    float2 offset;
    if (k >= 0.0) {
        offset = tile.yz * scale - target_tile.yz;
    } else {
        offset = (tile.yz - target_tile.yz * exp2(-k)) * scale;
    }
    clip.x = clip.x * scale + (scale - 1.0 + 2.0 * offset.x);
    clip.y = clip.y * scale + (1.0 - scale - 2.0 * offset.y);
    return clip;
}

enum {
    idGlobalPaintParamsUBO,
    idGlobalUBOIndex,
    globalUBOCount,
};

enum {
    idDrawableReservedVertexOnlyUBO = globalUBOCount,
    idDrawableReservedFragmentOnlyUBO,
    drawableReservedUBOCount
};

)";

}
} // namespace mbgl
