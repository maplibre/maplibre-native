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
    Default,  ///< Default TileLOD algorithm
    Distance, ///< Distance-based TileLOD algorithm
    /// Per-tile zoom from the field of view and a tile-count budget, rather than a
    /// single zoom for the whole cover: finer near the camera, coarser toward the
    /// horizon. This is maplibre-gl-js `createCalculateTileZoomFunction`, which GL JS
    /// applies whenever terrain is present or the pitch passes 78.5 - fov/2 degrees, so
    /// selecting it is how a terrain map matches GL JS's tile selection. Unlike
    /// `Distance` it varies the zoom at every pitch, and it ignores `TileLodScale`,
    /// `TileLodMinRadius`, `TileLodZoomShift` and `TileLodPitchThreshold`.
    Adaptive
};

/// Controls progressive loading of 3D-terrain content (draped tiles and drape
/// re-renders) and how much terrain mesh a frame may have. It trades initial-load
/// sharpness for smoother interaction on weaker GPUs, so it is a per-map, hardware-driven
/// choice. Default is Quality (no per-frame budget), which preserves the historical sharp
/// snap-load behaviour.
///
/// The mode also changes the final image: how deep the terrain mesh goes past a coarse DEM
/// (TerrainLoadBudget::meshTargetZoom) sets how sharp draped content stays in close-ups, and
/// how many mesh tiles a frame may have (TerrainLoadBudget::maxMeshTiles) sets how much
/// detail tilted views keep in the distance.
enum class TerrainLoadMode : uint8_t {
    Quality,     ///< No per-frame budget: all revealed tiles/drapes build immediately. Sharp, may
                 ///< stall a frame on big bursts (zoom-in over new coverage). The mesh follows
                 ///< the view past a coarser DEM, as maplibre-gl-js does, up to 64 mesh tiles.
                 ///< Default.
    Balanced,    ///< Cap 32 new-tile builds + 16 drape re-renders per frame. Mesh to z15, up to
                 ///< 48 mesh tiles.
    Performance, ///< Cap 8 new-tile builds + 4 drape re-renders per frame; smoothest on
                 ///< weak GPUs, most progressive fill-in. Mesh to z14, up to 24 mesh tiles.
};

/// Budgets for a TerrainLoadMode. A per-frame value <= 0 means unlimited.
struct TerrainLoadBudget {
    int newTileBuildsPerFrame;  ///< max NEW tiles that build their drawables per frame
    int drapeRerendersPerFrame; ///< max terrain drape targets that re-render per frame
    /// Hard cap on terrain mesh tiles per frame (0 = unlimited). When the cover exceeds it, the
    /// farthest tiles past the DEM's maxzoom first go back to coarser ones (down to the DEM's
    /// maxzoom), and only then are the farthest - the horizon tiles a high tilt pulls in -
    /// dropped, keeping those nearest the camera. Everything downstream scales with this (mesh draws, drape
    /// targets and re-renders, depth instances), so it is the bluntest frame-time lever - and
    /// it costs terrain render *distance*, which is why Quality keeps a generous cap rather
    /// than the aggressive one Performance uses.
    size_t maxMeshTiles;
    /// The zoom the terrain mesh reaches over a coarser DEM (RenderTerrain::meshMaxZoom). Each
    /// mesh tile drapes into its own 1024 px target, so how sharp draped content stays in a
    /// close-up follows the zoom of the mesh tiles, not of the DEM; each level deeper also adds
    /// tiles to the near field, and drape memory with them. A DEM at or past this zoom meshes
    /// at its own maxzoom. Quality's 22 (util::DEFAULT_MAX_ZOOM, the deepest a map goes) sets no
    /// limit: the mesh follows the view as maplibre-gl-js's terrain does, and only the tile cap
    /// bounds it. The other modes stop at most RenderTerrain::maxMeshOverzoomLevels past the DEM.
    uint8_t meshTargetZoom;
};

constexpr TerrainLoadBudget terrainLoadBudget(TerrainLoadMode mode) {
    switch (mode) {
        case TerrainLoadMode::Balanced:
            return {
                .newTileBuildsPerFrame = 32, .drapeRerendersPerFrame = 16, .maxMeshTiles = 48, .meshTargetZoom = 15};
        case TerrainLoadMode::Performance:
            return {.newTileBuildsPerFrame = 8, .drapeRerendersPerFrame = 4, .maxMeshTiles = 24, .meshTargetZoom = 14};
        case TerrainLoadMode::Quality:
        default:
            // no per-frame budget, generous tile cap, mesh follows the view
            return {.newTileBuildsPerFrame = 0, .drapeRerendersPerFrame = 0, .maxMeshTiles = 64, .meshTargetZoom = 22};
    }
}

/// Controls the vertical skirts extruded from every terrain tile edge. They hide the
/// hairline gaps (stitches) between neighbouring tiles at different zoom levels, but show
/// as vertical artifacts where the map has a transparent background - so which one you
/// want is a per-map tradeoff. Default is Auto. Mirrors maplibre-gl-js
/// `MapOptions.terrainSkirtLength`.
enum class TerrainSkirtLength : uint8_t {
    Auto, ///< Skirt every tile edge by ~1/5 of the tile's width at the current zoom. Default.
    None, ///< Build no skirts at all: no vertical artifacts over a transparent background,
          ///< at the cost of visible stitches between tiles at different zoom levels.
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
