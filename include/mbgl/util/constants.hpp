#pragma once

#include <mbgl/util/chrono.hpp>
#include <mbgl/util/unitbezier.hpp>

#include <cmath>
#include <cstddef>

namespace mbgl {

namespace util {

constexpr double tileSize_D = 512;
constexpr uint16_t tileSize_I = 512;

/*
 * The maximum extent of a feature that can be safely stored in the buffer.
 * In practice, all features are converted to this extent before being added.
 *
 * Positions are stored as signed 16bit integers.
 * One bit is lost for signedness to support features extending past the left
 * edge of the tile. One bit is lost because the line vertex buffer used to pack
 * 1 bit of other data into the int. This is no longer the case but we're
 * reserving this bit anyway. One bit is lost to support features extending past
 * the extent on the right edge of the tile. This leaves us with 2^13 = 8192
 */
constexpr int32_t EXTENT = 8192;

constexpr double M2PI = M_PI * 2;
constexpr double EARTH_RADIUS_M = 6378137;
constexpr double LATITUDE_MAX = 85.051128779806604;
constexpr double LONGITUDE_MAX = 180;
constexpr double DEGREES_MAX = 360;
constexpr double PITCH_MIN = 0.0;
constexpr double DEFAULT_PITCH_MAX = M_PI * 7.0 / 18.0;
constexpr double PITCH_MAX = M_PI * 7.0 / 18.0; 
// Default vertical field of view in radians.
constexpr double DEFAULT_FOV = 0.95;
// Solid sky color used above the horizon when the map is pitched (CSS "skyblue").
constexpr float DEFAULT_SKY_COLOR_R = 135.0f / 255.0f;
constexpr float DEFAULT_SKY_COLOR_G = 206.0f / 255.0f;
constexpr float DEFAULT_SKY_COLOR_B = 235.0f / 255.0f;
constexpr float DEFAULT_SKY_COLOR_A = 1.0f;
constexpr double MIN_PITCH_FOR_SKY = 0.01;
// NDC Y offset applied to the computed horizon line for sky rendering.
constexpr float SKY_HORIZON_NDC_OFFSET = 4.0f;
// Horizon / far-plane distance as multiples of camera altitude. Max scales up with pitch.
// Default: basemap tiles and symbol culling (slightly beyond 3D content).
constexpr double HORIZON_DISTANCE_MIN_MULTIPLIER = 6.0;
constexpr double HORIZON_DISTANCE_MAX_MULTIPLIER = 20.0;
// Fill extrusion and GLTF / custom 3D models share the same tighter far plane.
constexpr double HORIZON_FILL_EXTRUSION_MIN_MULTIPLIER = 2.5;
constexpr double HORIZON_FILL_EXTRUSION_MAX_MULTIPLIER = 12.0;
constexpr double MIN_ZOOM = 0.0;
constexpr double MAX_ZOOM = 25.5;
constexpr float MIN_ZOOM_F = MIN_ZOOM;
constexpr float MAX_ZOOM_F = MAX_ZOOM;
constexpr uint8_t DEFAULT_MAX_ZOOM = 22;

// ONE_EM constant used to go between "em" units used in style spec and "points" used internally for layout.
constexpr float ONE_EM = 24.0f;

// Normalized distance over which fill-extrusion fake bevel shading blends toward the up-vector.
constexpr float FILL_EXTRUSION_BEVEL_RADIUS = 0.07f;

constexpr uint8_t DEFAULT_PREFETCH_ZOOM_DELTA = 4;

constexpr uint64_t DEFAULT_MAX_CACHE_SIZE = 50 * 1024 * 1024;

// Default ImageManager's cache size for images added via onStyleImageMissing API.
// Average sprite size with 1.0 pixel ratio is ~2kB, 8kB for pixel ratio of 2.0.
constexpr std::size_t DEFAULT_ON_DEMAND_IMAGES_CACHE_SIZE = 100 * 8192;

constexpr Duration DEFAULT_TRANSITION_DURATION = Milliseconds(300);
constexpr Seconds CLOCK_SKEW_RETRY_TIMEOUT{30};

constexpr UnitBezier DEFAULT_TRANSITION_EASE = {0, 0, 0.25, 1};

constexpr int DEFAULT_RATE_LIMIT_TIMEOUT = 5;

constexpr const char* ASSET_PROTOCOL = "asset://";
constexpr const char* FILE_PROTOCOL = "file://";
constexpr const char* MBTILES_PROTOCOL = "mbtiles://";
constexpr const char* PMTILES_PROTOCOL = "pmtiles://";
constexpr uint32_t DEFAULT_MAXIMUM_CONCURRENT_REQUESTS = 20;

constexpr uint8_t TERRAIN_RGB_MAXZOOM = 15;

constexpr const char* LAST_RESORT_ALPHABETIC_FONT = "Open Sans Regular";
constexpr const char* LAST_RESORT_PAN_UNICODE_FONT = "Arial Unicode MS Regular";

} // namespace util

namespace debug {

extern const bool tileParseWarnings;
extern const bool styleParseWarnings;
extern const bool spriteWarnings;
extern const bool renderWarnings;
extern const bool labelTextMissingWarning;
extern const bool missingFontStackWarning;
extern const bool missingFontFaceWarning;
extern const bool glyphWarning;
extern const bool shapingWarning;

} // namespace debug

} // namespace mbgl
