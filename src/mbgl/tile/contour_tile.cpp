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
#include <optional>
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

// Sutherland-Hodgman-style polyline clip against an axis-aligned rect
// `[0, EXTENT] × [0, EXTENT]` in tile-local coords. Returns a list of
// sub-polylines covering only the interior portion of the input. Each
// sub-polyline starts and ends either at an interior vertex or at a
// segment-rect intersection point, so adjacent tiles' clipped lines
// share the boundary intersection exactly (both sides compute it from
// the same two MS vertices in the overlap region).
//
// Why this matters: Chaikin's corner-cutting preserves the first and
// last input vertex. If we trim each line so its endpoints land exactly
// on the tile boundary, the smoothed line in tile A and the smoothed
// line in tile B both terminate at the same boundary crossing point,
// so the rendered contours meet without a visible gap. Without trimming,
// each tile's smoothing window reached different vertices on the far
// side of the seam, producing slightly different smoothed curves and
// leaving misaligned ends.
std::vector<std::vector<algorithm::contour::Point2D>> clipPolylineToTile(
    const std::vector<algorithm::contour::Point2D>& line) {
    std::vector<std::vector<algorithm::contour::Point2D>> result;
    if (line.size() < 2) return result;

    constexpr double minX = 0.0;
    constexpr double minY = 0.0;
    constexpr double maxX = static_cast<double>(util::EXTENT);
    constexpr double maxY = static_cast<double>(util::EXTENT);
    constexpr double eps = 1e-9;

    auto isInside = [](const algorithm::contour::Point2D& p) {
        return p[0] >= minX - eps && p[0] <= maxX + eps && p[1] >= minY - eps && p[1] <= maxY + eps;
    };

    // Liang-Barsky line-rect intersection — return the parameter `t` (in
    // [0, 1]) at which segment a→b enters/exits the rect, or `nan` if
    // there's no intersection.
    auto intersectSegment = [&](const algorithm::contour::Point2D& a,
                                const algorithm::contour::Point2D& b,
                                bool wantEntry) -> std::optional<algorithm::contour::Point2D> {
        const double dx = b[0] - a[0];
        const double dy = b[1] - a[1];
        double tEnter = 0.0;
        double tExit = 1.0;
        const double p[4] = {-dx, dx, -dy, dy};
        const double q[4] = {a[0] - minX, maxX - a[0], a[1] - minY, maxY - a[1]};
        for (int i = 0; i < 4; i++) {
            if (p[i] == 0.0) {
                if (q[i] < 0.0) return std::nullopt;
            } else {
                const double t = q[i] / p[i];
                if (p[i] < 0.0) {
                    if (t > tExit) return std::nullopt;
                    if (t > tEnter) tEnter = t;
                } else {
                    if (t < tEnter) return std::nullopt;
                    if (t < tExit) tExit = t;
                }
            }
        }
        const double t = wantEntry ? tEnter : tExit;
        return algorithm::contour::Point2D{a[0] + t * dx, a[1] + t * dy};
    };

    std::vector<algorithm::contour::Point2D> current;
    current.reserve(line.size());
    bool prevInside = isInside(line[0]);
    if (prevInside) current.push_back(line[0]);

    for (std::size_t i = 1; i < line.size(); i++) {
        const auto& a = line[i - 1];
        const auto& b = line[i];
        const bool curInside = isInside(b);
        if (prevInside && curInside) {
            current.push_back(b);
        } else if (prevInside && !curInside) {
            if (auto exit = intersectSegment(a, b, false); exit) {
                current.push_back(*exit);
            }
            if (current.size() >= 2) result.push_back(std::move(current));
            current.clear();
        } else if (!prevInside && curInside) {
            if (auto entry = intersectSegment(a, b, true); entry) {
                current.push_back(*entry);
            }
            current.push_back(b);
        } else {
            // Both endpoints outside — segment may still cross the rect.
            // Test for an in-and-out crossing and emit it as a 2-vertex
            // sub-polyline.
            const double dx = b[0] - a[0];
            const double dy = b[1] - a[1];
            double tEnter = 0.0;
            double tExit = 1.0;
            const double p[4] = {-dx, dx, -dy, dy};
            const double q[4] = {a[0] - minX, maxX - a[0], a[1] - minY, maxY - a[1]};
            bool ok = true;
            for (int j = 0; j < 4 && ok; j++) {
                if (p[j] == 0.0) {
                    if (q[j] < 0.0) ok = false;
                } else {
                    const double t = q[j] / p[j];
                    if (p[j] < 0.0) {
                        if (t > tExit) ok = false;
                        else if (t > tEnter) tEnter = t;
                    } else {
                        if (t < tEnter) ok = false;
                        else if (t < tExit) tExit = t;
                    }
                }
            }
            if (ok && tEnter < tExit) {
                std::vector<algorithm::contour::Point2D> crossing;
                crossing.push_back({a[0] + tEnter * dx, a[1] + tEnter * dy});
                crossing.push_back({a[0] + tExit * dx, a[1] + tExit * dy});
                result.push_back(std::move(crossing));
            }
        }
        prevInside = curInside;
    }
    if (current.size() >= 2) result.push_back(std::move(current));
    return result;
}

// Compute the `natural_interval` for a line at elevation `elev` (display
// units): the largest entry in the source's distinct interval list that
// divides `elev`. `scheduleIntervals` must be sorted descending and contain
// only positive values.
//
// Lets style filters target a specific schedule band without modulo
// arithmetic — `["==", ["get", "natural_interval"], 200]` matches every
// line whose elevation is a multiple of the largest schedule interval
// (e.g. 200 m for a metres schedule of `[200, …, 100, …, 50, …, 20, …, 10]`).
// Whichever canonical zoom's tile is currently rendered, the same set of
// lines satisfies the filter, so a single style layer can fade them in
// across one display-zoom range without losing them at canonical-zoom
// transitions.
std::int64_t naturalInterval(std::int64_t elev,
                             const std::vector<double>& scheduleIntervals) {
    for (double v : scheduleIntervals) {
        const auto candidate = static_cast<std::int64_t>(std::llround(v));
        if (candidate <= 0) continue;
        if (elev % candidate == 0) return candidate;
    }
    return 0;
}

// Convert raw marching-squares output (interleaved int32 tile-local coords
// with elevation in metres) to vector-tile features.
//
// Per-feature properties:
//   ele              — elevation in display units (rounded int). Canonical
//                      style-spec property.
//   elevation        — duplicate of `ele` under the legacy property name
//                      used by some existing line/symbol styles. Engine-
//                      specific extra; consumers should prefer `ele`.
//   index            — Mapbox-Terrain-v2-compatible divisibility bucket
//                      (1 / 2 / 5 / 10) per `contourIndex` above. Engine-
//                      specific extra.
//   interval         — the contour spacing this tile was generated at, in
//                      display units. Reflects which canonical-zoom band
//                      the line came from; useful for diagnostics.
//   natural_interval — largest schedule interval that divides this line's
//                      elevation. Stable across canonical-zoom transitions
//                      (a line at ele=200 always has natural_interval=200
//                      whether it appears in the canonical-z=9 tile or the
//                      canonical-z=12 tile), so style layers can be
//                      filtered per band and faded smoothly.
//
// Pipeline per line:
//   1. Clip to the [0, EXTENT] tile bbox so endpoints land on the
//      boundary at the shared crossing point.
//   2. Douglas-Peucker simplification (drops sub-pixel wiggle so
//      Chaikin doesn't waste iterations on noise).
//   3. Chaikin corner-cutting (endpoints preserved by the algorithm,
//      and now also clamped to the tile boundary so adjacent tiles'
//      smoothed lines meet exactly).
//   4. Round to int16 MVT coords and emit.
mapbox::feature::feature_collection<std::int16_t> toFeatures(
    const std::vector<algorithm::contour::ContourLineString>& lines,
    const algorithm::contour::UnitConfig& unit,
    double intervalDisplayUnits,
    const std::vector<double>& scheduleIntervals) {
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

        for (auto& clipped : clipPolylineToTile(work)) {
            if (clipped.size() < 2) continue;
            const auto simplified = algorithm::contour::douglasPeucker(clipped, kSimplifyEpsilon);
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
            f.properties["interval"] = static_cast<std::int64_t>(std::llround(intervalDisplayUnits));
            f.properties["natural_interval"] = naturalInterval(elev, scheduleIntervals);
            features.push_back(std::move(f));
        }
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
                                  const std::vector<double>& scheduleIntervals,
                                  const algorithm::contour::UnitConfig& unit) {
    HillshadeBucket* bucket = demTile.getBucket();
    if (bucket == nullptr) return;
    const DEMData& dem = bucket->getDEMData();

    // Snapshot the buffered DEMData view on the render thread. The DEM
    // source uses a pixel-CENTER convention (verified empirically:
    // rightmost column of tile L and leftmost column of tile R differ
    // by ~one elevation cell, not zero), so each tile covers an
    // exclusive pixel range — sample 0 of tile L sits at world
    // west_L + 0.5·cellWidth, sample dim-1 at east_L - 0.5·cellWidth.
    //
    // We sample the entire (dim + 2·border) buffered view rather than
    // just the 1-pixel inner ring. With border = 2 the marching-squares
    // skirt extends two cells past the tile edge on every side, which
    // gives the downstream Douglas-Peucker / Chaikin smoothing more
    // context near the boundary so adjacent tiles' smoothed contours
    // share a tangent direction at the seam, not just an endpoint.
    // Adjacent tiles' overlapping output regions are clipped during
    // rendering, so the extra width costs only a small per-tile
    // generation step.
    //
    // Snapshot here on the render thread because backfillBorder mutates
    // the same image asynchronously when more neighbours arrive.
    const int dim = dem.dim;
    const int border = DEMData::border;
    const int width = dim + 2 * border;
    auto heights = std::make_shared<std::vector<std::int16_t>>();
    heights->reserve(static_cast<std::size_t>(width) * width);
    for (int y = -border; y < dim + border; y++) {
        for (int x = -border; x < dim + border; x++) {
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
        [heights, width, dim, intervalMeters, intervalDisplayUnits, scheduleIntervals, unit]() {
            algorithm::contour::ContourThresholds thresholds;
            thresholds.interval = intervalMeters;
            // Pixel-center coordinate mapping. We want:
            //   alg index `border`     (sample 0, world west_L + 0.5cw) → tile-local +0.5cw
            //   alg index `border+dim-1` (sample dim-1, east_L − 0.5cw) → EXTENT − 0.5cw
            //   alg indices [0, border-1] and [border+dim, width-1]    → outside the tile,
            //                                                            in the neighbour's interior
            //
            // The algorithm internally uses
            //   multiplier = thresholds.extent / (width - 1)
            // and emits output at multiplier·index. We want
            //   multiplier = EXTENT / dim
            // so each cell spans one tile-local cell width, no matter how
            // wide the buffered view is. That gives
            //   thresholds.extent = EXTENT × (width - 1) / dim
            //
            // The boundary cell on each tile (heights[border-1..border]
            // on the west, heights[width-2..width-1] on the east, after
            // backfillBorder) straddles the tile edge using identical
            // neighbour data, so adjacent tiles' contour features meet
            // exactly at the seam.
            thresholds.extent = static_cast<int>(std::lround(static_cast<double>(util::EXTENT) *
                                                             static_cast<double>(width - 1) /
                                                             static_cast<double>(dim)));
            thresholds.buffer = 0;
            const auto lines = algorithm::contour::generateContours(*heights, width, width, thresholds);

            // Shift so alg index `border` lands at +0.5cw (sample 0's
            // pixel-center position). At border = 1 this is −0.5cw.
            // At border = 2 it is −1.5cw — alg index 0 (outer west
            // border) sits one cell further west than the inner border
            // cell did before.
            const double halfCellPx = 0.5 * static_cast<double>(util::EXTENT) / static_cast<double>(dim);
            const double shift = halfCellPx * static_cast<double>(2 * DEMData::border - 1);
            std::vector<algorithm::contour::ContourLineString> shifted;
            shifted.reserve(lines.size());
            for (const auto& line : lines) {
                algorithm::contour::ContourLineString s;
                s.elevation = line.elevation;
                s.points.reserve(line.points.size());
                for (std::size_t i = 0; i + 1 < line.points.size(); i += 2) {
                    s.points.push_back(static_cast<std::int32_t>(std::lround(line.points[i] - shift)));
                    s.points.push_back(static_cast<std::int32_t>(std::lround(line.points[i + 1] - shift)));
                }
                shifted.push_back(std::move(s));
            }
            return toFeatures(shifted, unit, intervalDisplayUnits, scheduleIntervals);
        },
        [self = weakFactory.makeWeakPtr(), this](mapbox::feature::feature_collection<std::int16_t> features) {
            if (auto guard = self.lock(); self) {
                setData(std::make_unique<GeoJSONTileData>(std::move(features)));
            }
        });
}

} // namespace mbgl
