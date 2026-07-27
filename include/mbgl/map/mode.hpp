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
    Balanced,    ///< Cap 32 new-tile builds + 16 drape re-renders per frame, and cover the
                 ///< terrain a half zoom coarser (~30% fewer tiles on average). For mid-tier
                 ///< devices: a gentle draw-call cut with barely-perceptible detail loss.
    Performance, ///< Cap 8 new-tile builds + 4 drape re-renders per frame, and cover the
                 ///< terrain one zoom coarser (~1/4 the mesh tiles + drape targets). Aimed
                 ///< at CPU-encode-bound low-end GPUs, where draw-call count - not fill -
                 ///< caps the frame rate; trades relief/drape sharpness for fewer draw calls.
};

/// Per-frame budgets for a TerrainLoadMode. A value <= 0 means unlimited.
struct TerrainLoadBudget {
    int newTileBuildsPerFrame;  ///< max NEW tiles that build their drawables per frame
    int drapeRerendersPerFrame; ///< max terrain drape targets that re-render per frame
    /// Shift applied to the terrain mesh/drape cover zoom (added to the LOD zoom). Negative
    /// coarsens the cover: -1 ~= a quarter of the tiles, so a quarter of the terrain draw
    /// calls and drape targets, which is the dominant per-frame CPU-encode cost on weak
    /// devices. 0 keeps the view's full ideal detail.
    float coverZoomShift;
};

constexpr TerrainLoadBudget terrainLoadBudget(TerrainLoadMode mode) {
    switch (mode) {
        case TerrainLoadMode::Balanced:
            return {32, 16, -0.5f};
        case TerrainLoadMode::Performance:
            return {8, 4, -1.0f};
        case TerrainLoadMode::Quality:
        default:
            return {0, 0, 0.0f}; // unlimited, full detail
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
