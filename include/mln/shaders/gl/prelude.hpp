// Generated code, do not modify this file!
#pragma once
#include <mln/shaders/shader_source.hpp>

namespace mln {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::Prelude, gfx::Backend::Type::OpenGL> {
    static constexpr const char* name = "Prelude";
    static constexpr const char* vertex = R"(#ifdef GL_ES
precision highp float;
#else

#if !defined(lowp)
#define lowp
#endif

#if !defined(mediump)
#define mediump
#endif

#if !defined(highp)
#define highp
#endif

#endif

const float PI = 3.141592653589793;

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

// The offset depends on how many pixels are between the world origin and the edge of the tile:
// vec2 offset = mod(pixel_coord, size)
//
// At high zoom levels there are a ton of pixels between the world origin and the edge of the tile.
// The glsl spec only guarantees 16 bits of precision for highp floats. We need more than that.
//
// The pixel_coord is passed in as two 16 bit values:
// pixel_coord_upper = floor(pixel_coord / 2^16)
// pixel_coord_lower = mod(pixel_coord, 2^16)
//
// The offset is calculated in a series of steps that should preserve this precision:
vec2 get_pattern_pos(const vec2 pixel_coord_upper, const vec2 pixel_coord_lower,
    const vec2 pattern_size, const float tile_units_to_pixels, const vec2 pos) {

    vec2 offset = mod(mod(mod(pixel_coord_upper, pattern_size) * 256.0, pattern_size) * 256.0 + pixel_coord_lower, pattern_size);
    return (tile_units_to_pixels * pos + offset) / pattern_size;
}

// Sample the terrain elevation in meters at a tile-local coordinate, with manual
// bilinear interpolation on DEM pixel centers (the DEM has a 2px backfilled border),
// as in the maplibre-gl-js get_elevation() prelude function. Unlike gl-js (global
// terrain uniforms), MapLibre Native carries the DEM data per-drawable, so the DEM
// sampler and dem_* values are passed in as arguments rather than read from globals.
float get_elevation(vec2 pos, sampler2D dem, vec4 dem_coords, vec4 dem_unpack,
                    float dem_dim, float dem_exaggeration, float dem_enabled) {
    if (dem_enabled == 0.0) {
        return 0.0;
    }
    vec2 coord = (pos * dem_coords.x + dem_coords.yz) * dem_dim + 2.0;
    vec2 f = fract(coord);
    vec2 c = (floor(coord) + 0.5) / (dem_dim + 4.0);
    float d = 1.0 / (dem_dim + 4.0);
    vec4 tl = texture(dem, c) * 255.0;
    tl.a = -1.0;
    vec4 tr = texture(dem, c + vec2(d, 0.0)) * 255.0;
    tr.a = -1.0;
    vec4 bl = texture(dem, c + vec2(0.0, d)) * 255.0;
    bl.a = -1.0;
    vec4 br = texture(dem, c + vec2(d, d)) * 255.0;
    br.a = -1.0;
    float elevation = mix(mix(dot(tl, dem_unpack), dot(tr, dem_unpack), f.x),
                          mix(dot(bl, dem_unpack), dot(br, dem_unpack), f.x),
                          f.y);
    return elevation * dem_exaggeration;
}

// Like get_elevation, but samples the DEM from one layer of a sampler2DArray, for the
// instanced terrain depth pass (one draw covers many tiles, each with its own DEM layer).
// Identical bilinear terrarium/mapbox decode; the layer is selected per instance.
float get_elevation_array(vec2 pos, highp sampler2DArray dem, float layer, vec4 dem_coords,
                          vec4 dem_unpack, float dem_dim, float dem_exaggeration) {
    vec2 coord = (pos * dem_coords.x + dem_coords.yz) * dem_dim + 2.0;
    vec2 f = fract(coord);
    vec2 c = (floor(coord) + 0.5) / (dem_dim + 4.0);
    float d = 1.0 / (dem_dim + 4.0);
    vec4 tl = texture(dem, vec3(c, layer)) * 255.0;
    tl.a = -1.0;
    vec4 tr = texture(dem, vec3(c + vec2(d, 0.0), layer)) * 255.0;
    tr.a = -1.0;
    vec4 bl = texture(dem, vec3(c + vec2(0.0, d), layer)) * 255.0;
    bl.a = -1.0;
    vec4 br = texture(dem, vec3(c + vec2(d, d), layer)) * 255.0;
    br.a = -1.0;
    float elevation = mix(mix(dot(tl, dem_unpack), dot(tr, dem_unpack), f.x),
                          mix(dot(bl, dem_unpack), dot(br, dem_unpack), f.x),
                          f.y);
    return elevation * dem_exaggeration;
}

// Place a clip-space position computed with a tile-local drape matrix into the
// current terrain drape render target. `matrix` is the drawable's tile-local
// orthographic matrix with the drawable's tile (z, x, y) stored in its unused
// third column, and `target_tile` is GlobalPaintParamsUBO's drape_tile (the
// render target's tile, w != 0 while drawing into a drape target). The drape
// projection is orthographic, so placing a tile that overlaps the target at a
// different zoom is an affine transform in NDC. Factored so the offsets stay
// exact in single precision for tile coordinates up to zoom ~22.
vec4 apply_drape_transform(vec4 clip, mat4 matrix, vec4 target_tile) {
    if (target_tile.w == 0.0) {
        return clip;
    }
    vec3 tile = vec3(matrix[2][0], matrix[2][1], matrix[2][2]);
    float k = target_tile.x - tile.x; // target zoom - drawable zoom
    float scale = exp2(k);
    vec2 offset;
    if (k >= 0.0) {
        offset = tile.yz * scale - target_tile.yz;
    } else {
        offset = (tile.yz - target_tile.yz * exp2(-k)) * scale;
    }
    clip.x = clip.x * scale + (scale - 1.0 + 2.0 * offset.x);
    clip.y = clip.y * scale + (1.0 - scale - 2.0 * offset.y);
    return clip;
}

// Unpack a depth value packed by the terrain depth pass (terrain_depth.fragment.glsl): four
// base-256 digits of the window depth, each stored exactly as k / 255 in the RGBA8 target, so
// the decode is exact to float precision (the gl-js fract/bit_mask scheme stores k / 256 into a
// unorm8 that holds n / 255, an error of up to 0.002 - far more than the depth differences the
// test looks for at native's 1 px near plane). Converted back to clip-space NDC z to match the
// pos.z / pos.w compared in calculate_visibility.
float unpack_depth(vec4 rgba_depth) {
    const highp vec4 weights = vec4(1.0 / 256.0, 1.0 / 65536.0, 1.0 / 16777216.0, 1.0 / 4294967296.0);
    highp vec4 digits = floor(rgba_depth * 255.0 + 0.5);
    return dot(digits, weights) * 2.0 - 1.0;
}

// Opacity of a fragment behind the terrain, in [0, 1]: 1 fully visible, 0 fully hidden,
// with a soft ramp and a small bias so geometry sitting exactly on the terrain surface
// (e.g. a label anchored to it) does not occlude itself. Matches maplibre-gl-js depthOpacity().
// `depth_ramp` scales the gl-js ramp (500 per NDC unit) to this projection: gl-js uses a near
// plane of height / 50 where native's TransformState::getProjMatrix uses 1 (one pixel at the
// view centre), which compresses every fragment's NDC depth into the top ~0.1%. Scaling by the
// near-plane ratio keeps gl-js's behaviour (fully hidden ~0.11 view-heights behind the surface
// at the centre distance, self-occlusion bias ~5 px).
highp float depth_opacity(vec3 frag, sampler2D depth_texture, highp float depth_ramp) {
    highp float bias = 0.05 / depth_ramp; // gl-js: 0.0001 * 500
    highp float d = unpack_depth(texture(depth_texture, frag.xy * 0.5 + 0.5)) + bias - frag.z;
    return 1.0 - max(0.0, min(1.0, -d * depth_ramp));
}

// Whether a clip-space position is visible in front of the terrain, from the
// packed terrain depth texture, matching maplibre-gl-js calculate_visibility().
// Unlike gl-js (global terrain uniforms), the depth sampler and enable flag are
// passed as arguments. `camera_to_center_distance` is 1.5 x the view height in
// native's projection, so the near-plane ratio (height / 50) gives a ramp of
// 500 * height / 50 = 10 * height = 6.67 * camera_to_center_distance.
float calculate_visibility(vec4 pos, sampler2D depth_texture, float depth_enabled, highp float camera_to_center_distance) {
    if (depth_enabled == 0.0) {
        return 1.0;
    }
    highp float depth_ramp = 6.67 * camera_to_center_distance;
    vec3 frag = pos.xyz / pos.w;
    highp float d = depth_opacity(frag, depth_texture, depth_ramp);
    if (d > 0.95) {
        return 1.0;
    }
    // a label whose anchor is just behind a ridge still shows if its glyphs poke above it
    return (d + depth_opacity(frag + vec3(0.0, 0.01, 0.0), depth_texture, depth_ramp)) / 2.0;
}
)";
    static constexpr const char* fragment = R"(#ifdef GL_ES
precision mediump float;
#else

#if !defined(lowp)
#define lowp
#endif

#if !defined(mediump)
#define mediump
#endif

#if !defined(highp)
#define highp
#endif

#endif

out highp vec4 fragColor;
)";
};

} // namespace shaders
} // namespace mln
