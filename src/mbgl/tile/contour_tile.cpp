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

    // Snapshot the inner dim×dim heights on the render thread. The DEMData
    // image is shared with backfillBorder which mutates the 1-pixel border
    // when neighbour tiles arrive — we'd race if we sampled from the
    // background. Snapshotting up front (~0.5 MB worst case for our
    // pipeline's tileSize=512) is simpler and safer than coordinating reads.
    const int dim = dem.dim;
    auto heights = std::make_shared<std::vector<std::int16_t>>();
    heights->reserve(static_cast<std::size_t>(dim) * dim);
    for (int y = 0; y < dim; y++) {
        for (int x = 0; x < dim; x++) {
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
        [heights, dim, intervalMeters, unit]() {
            algorithm::contour::ContourThresholds thresholds;
            thresholds.interval = intervalMeters;
            thresholds.extent = util::EXTENT;
            thresholds.buffer = 0;
            const auto lines = algorithm::contour::generateContours(*heights, dim, dim, thresholds);
            return toFeatures(lines, unit);
        },
        [self = weakFactory.makeWeakPtr(), this](mapbox::feature::feature_collection<std::int16_t> features) {
            if (auto guard = self.lock(); self) {
                setData(std::make_unique<GeoJSONTileData>(std::move(features)));
            }
        });
}

} // namespace mbgl
