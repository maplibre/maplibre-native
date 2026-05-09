#include <mbgl/tile/contour_tile.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/algorithm/contour/isolines.hpp>
#include <mbgl/algorithm/contour/smoothing.hpp>
#include <mbgl/algorithm/contour/units.hpp>
#include <mbgl/geometry/dem_data.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/tile/geojson_tile_data.hpp>
#include <mbgl/tile/raster_dem_tile.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/identity.hpp>

#include <mapbox/feature.hpp>
#include <mapbox/geometry.hpp>

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace mbgl {

namespace {

// Mapbox-Terrain-v2-compatible `index` bucket, emitted as a per-feature
// property so styles can filter minor / intermediate / major / super-major
// lines without expressing the divisibility ladder in `["%", ["get", "ele"], …]`
// expressions. Highest applicable bucket wins:
//
//   metres elevation:
//     multiple of 500 → 10  (super-major; label candidate)
//     multiple of 100 → 5   (major)
//     multiple of 50  → 2   (intermediate)
//     otherwise       → 1   (minor)
//
//   feet elevation:
//     multiple of 1000 → 10
//     multiple of 500  → 5
//     multiple of 100  → 2
//     otherwise        → 1
//
// `elev = 0` is divisible by everything and returns the highest bucket — sea
// level renders as super-major. Custom unit has no natural divisibility
// ladder, so always returns 1 (minor). The thresholds are conventions, not
// part of the style spec; consumers wanting different cutoffs should
// compute their own bucket via expression filters.
std::int64_t contourIndex(std::int64_t elev, algorithm::contour::ContourUnit unit) {
    switch (unit) {
    case algorithm::contour::ContourUnit::Meters:
        if (elev % 500 == 0) return 10;
        if (elev % 100 == 0) return 5;
        if (elev % 50 == 0) return 2;
        return 1;
    case algorithm::contour::ContourUnit::Feet:
        if (elev % 1000 == 0) return 10;
        if (elev % 500 == 0) return 5;
        if (elev % 100 == 0) return 2;
        return 1;
    case algorithm::contour::ContourUnit::Custom:
        return 1;
    }
    return 1;
}

// Half-pixel-in-tile-local-units, used as the Douglas-Peucker simplification
// tolerance. With `util::EXTENT = 8192` across a 512-px MapLibre vector
// tile, one device pixel is 16 tile-local units; half a pixel is 8.
// Sub-pixel wiggle gets dropped before Chaikin's corner-cutting so the
// smoothing pass doesn't waste iterations on noise.
constexpr double kSimplifyEpsilon = 8.0;

// Two iterations of Chaikin gives a noticeable smoothing of the
// marching-squares staircase without ballooning the vertex count
// (each iteration roughly doubles vertices). Empirically a good
// quality / size balance.
constexpr int kChaikinIterations = 2;

// Convert raw marching-squares output (interleaved int32 tile-local coords
// with elevation in metres) to vector-tile features. Each line goes through Douglas-Peucker
// simplification then Chaikin smoothing before being rounded to int16 MVT
// coordinates.
mapbox::feature::feature_collection<std::int16_t> toFeatures(
    const std::vector<algorithm::contour::ContourLineString>& lines,
    const algorithm::contour::UnitConfig& unit) {
    mapbox::feature::feature_collection<std::int16_t> features;
    features.reserve(lines.size());

    std::vector<algorithm::contour::Point2D> work;
    for (const auto& line : lines) {
        if (line.points.size() < 4) continue;

        work.clear();
        work.reserve(line.points.size() / 2);
        for (std::size_t i = 0; i + 1 < line.points.size(); i += 2) {
            work.push_back({static_cast<double>(line.points[i]), static_cast<double>(line.points[i + 1])});
        }

        // Smooth: Douglas-Peucker simplification, then Chaikin corner-cutting.
        const auto simplified = algorithm::contour::douglasPeucker(work, kSimplifyEpsilon);
        if (simplified.size() < 2) continue;
        const auto smoothed = algorithm::contour::chaikin(simplified, kChaikinIterations);
        if (smoothed.size() < 2) continue;

        mapbox::geometry::line_string<std::int16_t> ls;
        ls.reserve(smoothed.size());
        for (const auto& p : smoothed) {
            ls.push_back({static_cast<std::int16_t>(std::lround(p[0])),
                          static_cast<std::int16_t>(std::lround(p[1]))});
        }

        // Round elevation to integer display units before emission.
        const double elevDisplay = algorithm::contour::metersToUnit(line.elevation, unit);
        const std::int64_t elev = static_cast<std::int64_t>(std::llround(elevDisplay));

        mapbox::feature::feature<std::int16_t> f;
        f.geometry = std::move(ls);
        f.properties["ele"] = elev;
        f.properties["elevation"] = elev;
        f.properties["index"] = contourIndex(elev, unit.unit);
        features.push_back(std::move(f));
    }
    return features;
}

} // namespace

ContourTile::ContourTile(const OverscaledTileID& id_,
                         std::string sourceID_,
                         const TileParameters& parameters,
                         TileObserver* observer_)
    : GeometryTile(id_, std::move(sourceID_), parameters, observer_) {}

void ContourTile::populateFromDEM(const RasterDEMTile& demTile,
                                  double intervalDisplayUnits,
                                  const algorithm::contour::UnitConfig& unit) {
    HillshadeBucket* bucket = demTile.getBucket();
    if (bucket == nullptr) return;
    const DEMData& dem = bucket->getDEMData();

    // Snapshot the (dim+2)×(dim+2) buffered DEMData view on the render
    // thread. Under the DEM source's pixel-CENTER convention (verified
    // empirically: rightmost column of tile L and leftmost column of
    // tile R differ by ~one elevation cell, not zero), each tile covers
    // an exclusive pixel range. Sample 0 of tile L sits at world
    // west_L + 0.5·cellWidth; sample dim-1 at east_L - 0.5·cellWidth.
    // The (dim+2) border, after backfillBorder runs, holds the
    // neighbour's actual sample at the cell beyond — so the boundary
    // cell straddles the tile edge with real data on both sides.
    //
    // Snapshot here on the render thread because backfillBorder mutates
    // the same image asynchronously when more neighbours arrive.
    const int dim = dem.dim;
    const int width = dim + 2;
    auto heights = std::make_shared<std::vector<std::int16_t>>();
    heights->reserve(static_cast<std::size_t>(width) * width);
    for (int y = -1; y <= dim; y++) {
        for (int x = -1; x <= dim; x++) {
            heights->push_back(static_cast<std::int16_t>(dem.get(x, y)));
        }
    }

    // The configured `intervalDisplayUnits` is in display units (feet,
    // metres, custom). Heights are always metres. Convert the threshold to
    // metres before running the algorithm so its level outputs come back in
    // metres too, then convert per-feature `ele` back to display units in
    // `toFeatures`.
    const double intervalMeters = algorithm::contour::unitToMeters(intervalDisplayUnits, unit);

    Scheduler::GetBackground()->scheduleAndReplyValue(
        util::SimpleIdentity::Empty,
        [heights, width, dim, intervalMeters, unit]() {
            algorithm::contour::ContourThresholds thresholds;
            thresholds.interval = intervalMeters;
            // Pixel-center coordinate mapping for a (dim+2) input. We want:
            //   alg index 0 (border, world west_L − 0.5cw) → tile-local −0.5cw_local
            //   alg index 1 (sample 0, world west_L + 0.5cw) → 0.5cw_local
            //   alg index dim (sample dim-1, east_L − 0.5cw) → EXTENT − 0.5cw_local
            //   alg index dim+1 (border, east_L + 0.5cw) → EXTENT + 0.5cw_local
            //
            // The algorithm internally uses
            //   multiplier = thresholds.extent / (width - 1) = thresholds.extent / (dim+1)
            // and produces output at multiplier·index. We want
            //   multiplier = EXTENT / dim
            // (so cw_local = EXTENT/dim and the half-cell shift below lines
            // up the boundary cell correctly). That gives
            //   thresholds.extent = EXTENT × (dim+1) / dim.
            //
            // Boundary cell L heights[dim..dim+1] and R heights[0..1] then
            // both straddle world east_L = west_R using identical neighbour
            // data, so adjacent tiles' contour features meet exactly.
            thresholds.extent = static_cast<int>(std::lround(static_cast<double>(util::EXTENT) *
                                                             static_cast<double>(dim + 1) /
                                                             static_cast<double>(dim)));
            thresholds.buffer = 0;
            const auto lines = algorithm::contour::generateContours(*heights, width, width, thresholds);

            // Half-cell shift so alg index 1 lands at +0.5cw (sample 0's
            // pixel-center position), with alg index 0 at -0.5cw (one cell
            // left of west_L) and alg index dim+1 at EXTENT + 0.5cw (one
            // cell right of east_L).
            const double halfCellPx = 0.5 * static_cast<double>(util::EXTENT) / static_cast<double>(dim);
            std::vector<algorithm::contour::ContourLineString> shifted;
            shifted.reserve(lines.size());
            for (const auto& line : lines) {
                algorithm::contour::ContourLineString s;
                s.elevation = line.elevation;
                s.points.reserve(line.points.size());
                for (std::size_t i = 0; i + 1 < line.points.size(); i += 2) {
                    s.points.push_back(static_cast<std::int32_t>(std::lround(line.points[i] - halfCellPx)));
                    s.points.push_back(static_cast<std::int32_t>(std::lround(line.points[i + 1] - halfCellPx)));
                }
                shifted.push_back(std::move(s));
            }
            return toFeatures(shifted, unit);
        },
        [self = weakFactory.makeWeakPtr(), this](mapbox::feature::feature_collection<std::int16_t> features) {
            if (auto guard = self.lock(); self) {
                setData(std::make_unique<GeoJSONTileData>(std::move(features)));
            }
        });
}

} // namespace mbgl
