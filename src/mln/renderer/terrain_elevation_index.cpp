#include <mln/renderer/terrain_elevation_index.hpp>

#include <mln/geometry/dem_data.hpp>
#include <mln/math/clamp.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/util/projection.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace mln {

TerrainElevationIndex::Grid::Grid(const DEMData& dem)
    : dim(dem.dim) {
    const int32_t side = dim + 1;
    heights.resize(static_cast<size_t>(side) * side);
    for (int32_t y = 0; y < side; ++y) {
        for (int32_t x = 0; x < side; ++x) {
            const int32_t h = util::clamp(dem.get(x, y),
                                          static_cast<int32_t>(std::numeric_limits<int16_t>::min()),
                                          static_cast<int32_t>(std::numeric_limits<int16_t>::max()));
            heights[static_cast<size_t>(y) * side + x] = static_cast<int16_t>(h);
        }
    }
}

TerrainElevationIndex::TerrainElevationIndex(std::vector<Tile> tiles_, float exaggeration_)
    : tiles(std::move(tiles_)),
      exaggeration(exaggeration_) {
    for (const auto& tile : tiles) {
        const uint8_t z = tile.id.canonical.z;
        auto level = std::find_if(levels.begin(), levels.end(), [z](const Level& l) { return l.z == z; });
        if (level == levels.end()) {
            levels.push_back(Level{z, {}});
            level = std::prev(levels.end());
        }
        const int64_t across = int64_t{1} << z;
        const int64_t x = static_cast<int64_t>(tile.id.wrap) * across + tile.id.canonical.x;
        level->tiles.emplace(key(x, tile.id.canonical.y), &tile);
    }
    std::sort(levels.begin(), levels.end(), [](const Level& a, const Level& b) { return a.z > b.z; });
}

std::optional<double> TerrainElevationIndex::getElevation(const LatLng& latLng) const {
    for (const auto& level : levels) {
        // The integer-zoom overload projects into tile units (world size 2^z), not pixels.
        const Point<double> world = Projection::project(latLng, level.z);
        const double fx = std::floor(world.x);
        const double fy = std::floor(world.y);
        const auto found = level.tiles.find(key(static_cast<int64_t>(fx), static_cast<int64_t>(fy)));
        if (found == level.tiles.end()) {
            continue;
        }
        // Bilinear interpolation of the DEM texels, as RenderTerrain::getElevation
        const Grid& grid = *found->second->grid;
        const auto dim = static_cast<double>(grid.dim);
        const double px = util::clamp((world.x - fx) * dim, 0.0, dim - 1.0);
        const double py = util::clamp((world.y - fy) * dim, 0.0, dim - 1.0);
        const auto x0 = static_cast<int32_t>(std::floor(px));
        const auto y0 = static_cast<int32_t>(std::floor(py));
        const double tx = px - x0;
        const double ty = py - y0;
        const double tl = grid.get(x0, y0);
        const double tr = grid.get(x0 + 1, y0);
        const double bl = grid.get(x0, y0 + 1);
        const double br = grid.get(x0 + 1, y0 + 1);
        const double top = tl + (tr - tl) * tx;
        const double bottom = bl + (br - bl) * tx;
        return (top + (bottom - top) * ty) * exaggeration;
    }
    return std::nullopt;
}

std::optional<LatLng> pickTerrainSurface(const TransformState& state,
                                         const ScreenCoordinate& pixel,
                                         const std::function<std::optional<double>(const LatLng&)>& elevationAt) {
    // TransformState unprojects y-up screen points; queries and the platform views hand us y-down
    ScreenCoordinate flipped = pixel;
    flipped.y = state.getSize().height - pixel.y;
    const LatLng seaLevel = state.screenCoordinateToLatLng(flipped);
    if (!elevationAt(seaLevel)) {
        return std::nullopt; // no DEM under the ray: nothing to pick against
    }
    const auto camera = state.getFreeCameraOptions().getLocation();
    if (!camera || camera->altitude <= 0.0) {
        return std::nullopt;
    }
    // Terrain height (exaggerated metres, the projection's z unit) where the ray crosses the
    // horizontal plane at `planeHeight`
    const auto surfaceHeightAt = [&](double planeHeight, LatLng& out) {
        out = state.screenCoordinateToLatLng(flipped, planeHeight);
        return elevationAt(out).value_or(0.0);
    };
    constexpr int steps = 64;
    const double top = camera->altitude * 0.999;
    double above = top; // last plane height known to be above the surface
    LatLng probe;
    if (surfaceHeightAt(above, probe) >= above) {
        return std::nullopt; // camera inside the terrain
    }
    for (int i = 1; i <= steps; ++i) {
        const double h = top * (1.0 - static_cast<double>(i) / steps);
        if (surfaceHeightAt(h, probe) < h) {
            above = h;
            continue;
        }
        // Crossed between `above` and `h`: bisect onto the surface
        double below = h;
        for (int j = 0; j < 8; ++j) {
            const double mid = 0.5 * (above + below);
            if (surfaceHeightAt(mid, probe) < mid) {
                above = mid;
            } else {
                below = mid;
            }
        }
        return state.screenCoordinateToLatLng(flipped, 0.5 * (above + below), LatLng::Wrapped);
    }
    return std::nullopt;
}

} // namespace mln
