/**
 * @file Contains enumerations for various modes
 */

#pragma once

#include <mln/util/util.hpp>
#include <mln/util/traits.hpp>

#include <cstddef>
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

/// Controls progressive loading of 3D-terrain content (draped tiles and drape
/// re-renders). It trades initial-load sharpness for smoother interaction on weaker
/// GPUs, so it is a per-map, hardware-driven choice. Default is Quality (no budget),
/// which preserves the historical sharp snap-load behaviour.
enum class TerrainLoadMode : uint8_t {
    Quality,     ///< No budget: all revealed tiles/drapes build immediately. Sharp, may
                 ///< stall a frame on big bursts (zoom-in over new coverage). Default.
    Balanced,    ///< Cap 32 new-tile builds + 16 drape re-renders per frame.
    Performance, ///< Cap 8 new-tile builds + 4 drape re-renders per frame; smoothest on
                 ///< weak GPUs, most progressive fill-in.
};

/// Per-frame budgets for a TerrainLoadMode. A value <= 0 means unlimited.
struct TerrainLoadBudget {
    int newTileBuildsPerFrame;  ///< max NEW tiles that build their drawables per frame
    int drapeRerendersPerFrame; ///< max terrain drape targets that re-render per frame
    /// Hard cap on terrain mesh tiles per frame (0 = unlimited). When the cover exceeds it the
    /// tiles nearest the map centre are kept and the farthest - the horizon tiles a high tilt
    /// pulls in - are dropped. Everything downstream scales with this (mesh draws, drape
    /// targets and re-renders, depth instances), so it is the bluntest frame-time lever - and
    /// it costs terrain render *distance*, which is why Quality keeps a generous cap rather
    /// than the aggressive one Performance uses.
    size_t maxMeshTiles;
};

constexpr TerrainLoadBudget terrainLoadBudget(TerrainLoadMode mode) {
    switch (mode) {
        case TerrainLoadMode::Balanced:
            return {32, 16, 48};
        case TerrainLoadMode::Performance:
            return {8, 4, 24};
        case TerrainLoadMode::Quality:
        default:
            return {0, 0, 64}; // no per-frame budget, generous tile cap
    }
}

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
