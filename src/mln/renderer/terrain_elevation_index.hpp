#pragma once

#include <mln/tile/tile_id.hpp>
#include <mln/util/geo.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace mln {

class DEMData;
class TransformState;

/// A read-only copy of the terrain heights the renderer has loaded, for the map side.
///
/// Only the render side holds the DEM, and on platforms where the map and the renderer run on
/// different threads (Android) asking it for a height means waiting on the render thread. The
/// renderer instead hands the map one of these whenever its set of DEM tiles changes, and
/// the map answers height questions from it straight away. It owns its data: the renderer's
/// DEMData lives in a bucket that is destroyed when its tile is evicted.
class TerrainElevationIndex {
public:
    /// One DEM tile's heights in whole metres (DEMData::get), `dim + 1` square including the
    /// right and bottom border texel that bilinear sampling reads at the tile edge.
    struct Grid {
        explicit Grid(const DEMData&);

        int32_t dim;
        std::vector<int16_t> heights;
        /// Highest of `heights`, unexaggerated
        int16_t maxHeight = 0;

        float get(int32_t x, int32_t y) const { return heights[static_cast<size_t>(y) * (dim + 1) + x]; }
    };

    struct Tile {
        UnwrappedTileID id;
        std::shared_ptr<const Grid> grid;
    };

    TerrainElevationIndex(std::vector<Tile> tiles, float exaggeration);
    // The zoom levels point into `tiles`
    TerrainElevationIndex(const TerrainElevationIndex&) = delete;
    TerrainElevationIndex& operator=(const TerrainElevationIndex&) = delete;

    /// Exaggerated terrain height in metres at `latLng`, sampled from the deepest tile that
    /// covers it, as RenderTerrain::getElevationForLatLng. Nullopt where no DEM tile is loaded.
    std::optional<double> getElevation(const LatLng& latLng) const;

    float getExaggeration() const { return exaggeration; }
    /// Exaggerated height in metres of the highest point in any of the tiles; where the terrain
    /// pick can start its march (pickTerrainSurface).
    double getMaxElevation() const { return maxElevation; }
    const std::vector<Tile>& getTiles() const { return tiles; }

private:
    struct Level {
        uint8_t z;
        /// Keyed by (unwrapped x, y) packed into 64 bits
        std::unordered_map<uint64_t, const Tile*> tiles;
    };

    static uint64_t key(int64_t x, int64_t y) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint32_t>(y);
    }

    std::vector<Tile> tiles;
    /// Deepest zoom first, so the first hit is the finest data
    std::vector<Level> levels;
    float exaggeration;
    double maxElevation = 0.0;
};

/// Coordinate of the terrain surface under a screen pixel (y-down view pixels, as rendered-feature
/// queries use), given the terrain height at a coordinate. The pixel's view ray is marched down from
/// just above `maxElevation` (or from just below the camera, if that is lower) to sea level, sampling
/// the height at each candidate plane, and the first crossing (the nearest surface, so a ridge in
/// front wins) is bisected; a coordinate without a height counts as sea level. Starting at the
/// highest terrain spreads the march's steps over the heights the surface can be at, so a thin ridge
/// is not stepped over at a grazing angle. Inverse of Map::pixelForLatLng(latLng, elevation).
/// nullopt without a camera altitude, when the camera is inside the terrain, or when the ray never
/// meets the surface above sea level. Shared by RenderTerrain::pickLatLng and the map side's
/// TerrainElevationIndex.
std::optional<LatLng> pickTerrainSurface(const TransformState& state,
                                         const ScreenCoordinate& pixel,
                                         const std::function<std::optional<double>(const LatLng&)>& elevationAt,
                                         double maxElevation);

} // namespace mln
