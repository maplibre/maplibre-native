/**
 * @file Contains enumerations for various modes
 */

#pragma once

#include <mln/util/util.hpp>
#include <mln/util/traits.hpp>

#include <cstdint>

namespace mln {

using EnumType = uint32_t;

enum class MapMode : EnumType {
    Continuous, ///< continually updating map
    Static,     ///< a once-off still image of an arbitrary viewport
    Tile        ///< a once-off still image of a single tile
};

/// We can choose to constrain the map both horizontally or vertically, only
/// vertically e.g. while panning, or screen to the specified bounds.
enum class ConstrainMode : EnumType {
    None,
    HeightOnly,
    WidthAndHeight,
    Screen,
};

/// Satisfies embedding platforms that requires the viewport coordinate systems
/// to be set according to its standards.
enum class ViewportMode : EnumType {
    Default,
    FlippedY,
};

enum class TileLodMode : uint8_t {
    Default, ///< Default TileLOD algorithm
    Distance ///< Distance-based TileLOD algorithm
};

/// Controls drawing tiles from the in-memory tile cache in place of ideal tiles
/// that are still loading. See `Map::setCachedTileFallbackOptions`.
struct CachedTileFallbackOptions {
    /// Whether the fallback is enabled. Off by default.
    bool enabled = false;
    /// How many zoom levels above the ideal tile to look for a cached parent tile.
    /// The search also stops at the source's minimum zoom.
    uint8_t maxParentZoomDelta = 255;
    /// How many zoom levels below the ideal tile to look for cached child tiles.
    uint8_t maxChildZoomDelta = 2;

    bool operator==(const CachedTileFallbackOptions&) const = default;
};

enum class MapDebugOptions : EnumType {
    NoDebug = 0,
    TileBorders = 1 << 1,
    ParseStatus = 1 << 2,
    Timestamps = 1 << 3,
    Collision = 1 << 4,
    Overdraw = 1 << 5,
    StencilClip = 1 << 6,
    DepthBuffer = 1 << 7,
};

constexpr MapDebugOptions operator|(MapDebugOptions lhs, MapDebugOptions rhs) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return MapDebugOptions(mln::underlying_type(lhs) | mln::underlying_type(rhs));
}

constexpr MapDebugOptions& operator|=(MapDebugOptions& lhs, MapDebugOptions rhs) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return (lhs = MapDebugOptions(mln::underlying_type(lhs) | mln::underlying_type(rhs)));
}

constexpr bool operator&(MapDebugOptions lhs, MapDebugOptions rhs) {
    return mln::underlying_type(lhs) & mln::underlying_type(rhs);
}

constexpr MapDebugOptions& operator&=(MapDebugOptions& lhs, MapDebugOptions rhs) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return (lhs = MapDebugOptions(mln::underlying_type(lhs) & mln::underlying_type(rhs)));
}

constexpr MapDebugOptions operator~(MapDebugOptions value) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return MapDebugOptions(~mln::underlying_type(value));
}

} // namespace mln
