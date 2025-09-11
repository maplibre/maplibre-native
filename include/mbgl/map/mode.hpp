/**
 * @file Contains enumerations for various modes
 */

#pragma once

#include <mbgl/util/util.hpp>
#include <mbgl/util/traits.hpp>

#include <cstdint>

namespace mbgl {

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
    return MapDebugOptions(mbgl::underlying_type(lhs) | mbgl::underlying_type(rhs));
}

constexpr MapDebugOptions& operator|=(MapDebugOptions& lhs, MapDebugOptions rhs) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return (lhs = MapDebugOptions(mbgl::underlying_type(lhs) | mbgl::underlying_type(rhs)));
}

constexpr bool operator&(MapDebugOptions lhs, MapDebugOptions rhs) {
    return mbgl::underlying_type(lhs) & mbgl::underlying_type(rhs);
}

constexpr MapDebugOptions& operator&=(MapDebugOptions& lhs, MapDebugOptions rhs) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return (lhs = MapDebugOptions(mbgl::underlying_type(lhs) & mbgl::underlying_type(rhs)));
}

constexpr MapDebugOptions operator~(MapDebugOptions value) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    return MapDebugOptions(~mbgl::underlying_type(value));
}

} // namespace mbgl
