#include <mbgl/tile/contour_tile.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/algorithm/contour/isolines.hpp>
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

// Convert raw marching-squares output (interleaved int32 tile-local coords
// with elevation in metres) to a vector-tile feature collection in
// display-unit attributes. `intervalMeters` is the threshold spacing in
// metres; `intervalDisplay` is the same in the configured display unit
// (used as the per-feature `interval` property).
mapbox::feature::feature_collection<std::int16_t> toFeatures(
    const std::vector<algorithm::contour::ContourLineString>& lines,
    double intervalMeters,
    double intervalDisplay,
    const algorithm::contour::UnitConfig& unit,
    std::uint32_t majorMultiplier) {
    mapbox::feature::feature_collection<std::int16_t> features;
    features.reserve(lines.size());
    for (const auto& line : lines) {
        if (line.points.size() < 4) continue;
        mapbox::geometry::line_string<std::int16_t> ls;
        ls.reserve(line.points.size() / 2);
        for (std::size_t i = 0; i + 1 < line.points.size(); i += 2) {
            ls.push_back({static_cast<std::int16_t>(line.points[i]),
                          static_cast<std::int16_t>(line.points[i + 1])});
        }

        // Step index: how many `intervalMeters` from zero this line sits.
        // Used for the major flag — every Nth step is major.
        const long long step = static_cast<long long>(std::llround(line.elevation / intervalMeters));
        const bool major = majorMultiplier > 0 && (step % static_cast<long long>(majorMultiplier)) == 0;

        mapbox::feature::feature<std::int16_t> f;
        f.geometry = std::move(ls);
        f.properties["ele"] = algorithm::contour::metersToUnit(line.elevation, unit);
        f.properties["interval"] = intervalDisplay;
        f.properties["major"] = major;
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
                                  const algorithm::contour::UnitConfig& unit,
                                  std::uint32_t majorMultiplier) {
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
        [heights, dim, intervalMeters, intervalDisplayUnits, unit, majorMultiplier]() {
            algorithm::contour::ContourThresholds thresholds;
            thresholds.interval = intervalMeters;
            thresholds.extent = util::EXTENT;
            thresholds.buffer = 0;
            const auto lines = algorithm::contour::generateContours(*heights, dim, dim, thresholds);
            return toFeatures(lines, intervalMeters, intervalDisplayUnits, unit, majorMultiplier);
        },
        [self = weakFactory.makeWeakPtr(), this](mapbox::feature::feature_collection<std::int16_t> features) {
            if (auto guard = self.lock(); self) {
                setData(std::make_unique<GeoJSONTileData>(std::move(features)));
            }
        });
}

} // namespace mbgl
