#pragma once

#include <mbgl/map/transform_state.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/util/range.hpp>

#include <vector>
#include <memory>
#include <numbers>
#include <optional>
#include <set>

namespace mbgl {

class LatLngBounds;

namespace util {

// Helper class to stream tile-cover results per row
class TileCover {
public:
    TileCover(const LatLngBounds&, uint8_t z);
    // When project == true, projects the geometry points to tile coordinates
    TileCover(const Geometry<double>&, uint8_t z, bool project = true);
    ~TileCover();

    std::optional<UnwrappedTileID> next();
    bool hasNext();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

/// Supplies the elevation range of a tile, so that the tile cover can test terrain
/// against the view frustum instead of the flat ground plane. Terrain rising towards
/// the camera occupies screen space that its flat footprint does not, so without this
/// a tile whose relief is in view is judged off-screen and never requested - which
/// leaves the terrain mesh with nothing to drape. Implemented over the DEM source's
/// loaded tiles; see maplibre-gl-js CoveringTilesOptions::terrain, which this mirrors.
class TileElevationProvider {
public:
    virtual ~TileElevationProvider() = default;

    /// The lowest and highest elevation within the tile, in meters, or nullopt when
    /// no DEM covering the tile is loaded (the caller then falls back to flat).
    virtual std::optional<Range<double>> getTileElevationRange(const CanonicalTileID&) const = 0;
};

struct TileCoverParameters {
    TransformState transformState;
    double tileLodMinRadius = 3;
    double tileLodScale = 1;
    double tileLodPitchThreshold = (60.0 / 180.0) * std::numbers::pi;
    TileLodMode tileLodMode = TileLodMode::Default;
    /// Optional; when null the cover is computed against the flat ground plane, as
    /// it was before terrain support.
    const TileElevationProvider* elevationProvider = nullptr;
};

int32_t coveringZoomLevel(double z, style::SourceType type, uint16_t tileSize) noexcept;

std::vector<OverscaledTileID> tileCover(const TileCoverParameters& state,
                                        uint8_t z,
                                        const Range<uint8_t> zoomRange,
                                        const std::optional<uint8_t>& overscaledZ = std::nullopt);

/// Keep only the tiles whose (elevation-aware) bounds intersect the view frustum.
/// The terrain mesh is built by subdividing the DEM source's render tiles, which
/// include large low-zoom ancestors when the DEM is sparse; most of such a tile's
/// area is off-screen, and meshing it creates drape targets no on-screen source
/// covers - so they render empty. Culling the mesh set to the frustum drops that
/// off-screen area. Uses params.elevationProvider the same way tileCover does.
std::set<UnwrappedTileID> frustumCull(const TileCoverParameters& state, const std::set<UnwrappedTileID>& tiles);
std::vector<UnwrappedTileID> tileCover(const LatLngBounds&, uint8_t z);
std::vector<UnwrappedTileID> tileCover(const Geometry<double>&, uint8_t z);

// Compute only the count of tiles needed for tileCover
uint64_t tileCount(const LatLngBounds&, uint8_t z) noexcept;
uint64_t tileCount(const Geometry<double>&, uint8_t z);

} // namespace util
} // namespace mbgl
