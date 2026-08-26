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

layout (std140) uniform ProjectionUBO {
    highp mat4 u_projection_matrix;
    highp mat4 u_projection_fallback_matrix;
    highp vec4 u_projection_tile_mercator_coords;
    highp vec4 u_projection_clipping_plane;
    highp float u_projection_transition;
    highp float u_projection_depth_offset;
    highp vec2 u_projection_translate;
};

// Pole vertices carry these sentinel Y values in their raw position.
#define GLOBE_POLE_NORTH_Y -32767.5
#define GLOBE_POLE_SOUTH_Y 32766.5

#ifdef PROJECTION_GLOBE

#define GLOBE_RADIUS 6371008.8
#define GLOBE_PI 3.1415926535897932384626433832795

// Tile position (0..EXTENT) to a point on the unit sphere; the pole sentinels in rawPos map to the poles.
vec3 projectToSphere(vec2 translatedPos, vec2 rawPos) {
    vec2 mercator_pos = u_projection_tile_mercator_coords.xy + u_projection_tile_mercator_coords.zw * translatedPos;
    float spherical_x = mercator_pos.x * GLOBE_PI * 2.0 + GLOBE_PI;
    // sin/cos of the latitude from the Mercator Y via the tangent half-angle identities: no atan, and float32 precision survives near the equator.
    float t = exp(GLOBE_PI - (mercator_pos.y * GLOBE_PI * 2.0));
    float t2 = t * t;
    float denom = t2 + 1.0;
    float sin_sy = (t2 - 1.0) / denom;
    float cos_sy = (2.0 * t) / denom;
    vec3 pos = vec3(sin(spherical_x) * cos_sy, sin_sy, cos(spherical_x) * cos_sy);
    if (rawPos.y < GLOBE_POLE_NORTH_Y) {
        pos = vec3(0.0, 1.0, 0.0);
    }
    if (rawPos.y > GLOBE_POLE_SOUTH_Y) {
        pos = vec3(0.0, -1.0, 0.0);
    }
    return pos;
}

vec3 globeRotateVector(vec3 vec, vec2 angles) {
    vec3 axisRight = vec3(vec.z, 0.0, -vec.x);
    vec3 axisUp = cross(axisRight, vec);
    axisRight = normalize(axisRight);
    axisUp = normalize(axisUp);
    vec2 t = tan(angles);
    return normalize(vec + axisRight * t.x + axisUp * t.y);
}

// cos(latitude) at a tile Y, from the same exp() form as projectToSphere.
float circumferenceRatioAtTileY(float tileY) {
    float mercator_pos_y = u_projection_tile_mercator_coords.y + u_projection_tile_mercator_coords.w * tileY;
    float t = exp(GLOBE_PI - (mercator_pos_y * GLOBE_PI * 2.0));
    return (2.0 * t) / (t * t + 1.0);
}

float projectLineThickness(float tileY) {
    float thickness = 1.0 / circumferenceRatioAtTileY(tileY);
    if (u_projection_transition < 0.999) {
        return mix(1.0, thickness, u_projection_transition);
    }
    return thickness;
}

float globeComputeClippingZ(vec3 spherePos) {
    return 1.0 - (dot(spherePos, u_projection_clipping_plane.xyz) + u_projection_clipping_plane.w);
}

vec4 interpolateProjection(vec2 posInTile, vec3 spherePos, float elevation) {
    vec3 elevatedPos = spherePos * (1.0 + elevation / GLOBE_RADIUS);
    vec4 globePosition = u_projection_matrix * vec4(elevatedPos, 1.0);
    // Clip the far side of the globe through Z; the layer's depth shift keeps layer order.
    globePosition.z = globeComputeClippingZ(elevatedPos) * globePosition.w - u_projection_depth_offset;

    if (u_projection_transition > 0.999) {
        return globePosition;
    }

    vec4 flatPosition = u_projection_fallback_matrix * vec4(posInTile, elevation, 1.0);
    const float z_globeness_threshold = 0.2;
    vec4 result = globePosition;
    result.z = mix(0.0, globePosition.z, clamp((u_projection_transition - z_globeness_threshold) / (1.0 - z_globeness_threshold), 0.0, 1.0));
    result.xyw = mix(flatPosition.xyw, globePosition.xyw, u_projection_transition);
    if ((posInTile.y < GLOBE_POLE_NORTH_Y) || (posInTile.y > GLOBE_POLE_SOUTH_Y)) {
        result = globePosition;
        const float poles_hidden_anim_percentage = 0.02;
        result.z = mix(globePosition.z, 100.0, pow(max((1.0 - u_projection_transition) / poles_hidden_anim_percentage, 0.0), 8.0));
    }
    return result;
}

// Keeps the matrix Z, for geometry that needs the depth buffer.
vec4 interpolateProjectionFor3D(vec2 posInTile, vec3 spherePos, float elevation) {
    vec3 elevatedPos = spherePos * (1.0 + elevation / GLOBE_RADIUS);
    vec4 globePosition = u_projection_matrix * vec4(elevatedPos, 1.0);
    if (u_projection_transition > 0.999) {
        return globePosition;
    }
    vec4 fallbackPosition = u_projection_fallback_matrix * vec4(posInTile, elevation, 1.0);
    return mix(fallbackPosition, globePosition, u_projection_transition);
}

vec4 projectTile(vec2 pos) {
    return interpolateProjection(pos, projectToSphere(pos + u_projection_translate, vec2(0.0, 0.0)), 0.0);
}

// The variant for geometry that can carry pole vertices; rawPos is the untranslated position.
vec4 projectTile(vec2 pos, vec2 rawPos) {
    return interpolateProjection(pos, projectToSphere(pos + u_projection_translate, rawPos), 0.0);
}

vec4 projectTileWithElevation(vec2 pos, float elevation) {
    return interpolateProjection(pos, projectToSphere(pos + u_projection_translate, vec2(0.0, 0.0)), elevation);
}

vec4 projectTileFor3D(vec2 pos, float elevation) {
    return interpolateProjectionFor3D(pos, projectToSphere(pos + u_projection_translate, pos), elevation);
}

#else

vec4 projectTile(vec2 pos) {
    return u_projection_matrix * vec4(pos, 0.0, 1.0);
}

// Pole vertices only exist on the globe; put them behind the near plane so their triangles are clipped.
vec4 projectTile(vec2 pos, vec2 rawPos) {
    vec4 result = u_projection_matrix * vec4(pos, 0.0, 1.0);
    if (rawPos.y < GLOBE_POLE_NORTH_Y || rawPos.y > GLOBE_POLE_SOUTH_Y) {
        result.z = -10000000.0;
    }
    return result;
}

vec4 projectTileWithElevation(vec2 pos, float elevation) {
    return u_projection_matrix * vec4(pos, elevation, 1.0);
}

vec4 projectTileFor3D(vec2 pos, float elevation) {
    return u_projection_matrix * vec4(pos, elevation, 1.0);
}

float projectLineThickness(float tileY) {
    return 1.0;
}

#endif
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
