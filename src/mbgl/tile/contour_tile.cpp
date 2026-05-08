#include <mbgl/tile/contour_tile.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/algorithm/contour/isolines.hpp>
#include <mbgl/geometry/dem_data.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/tile/geojson_tile_data.hpp>
#include <mbgl/tile/raster_dem_tile.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/identity.hpp>

#include <mapbox/feature.hpp>
#include <mapbox/geometry.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mbgl {

namespace {

// Convert raw marching-squares output (interleaved int32 tile-local coords) to
// a vector-tile feature collection in int16 coords (MVT extent fits in int16).
mapbox::feature::feature_collection<std::int16_t> toFeatures(
    const std::vector<algorithm::contour::ContourLineString>& lines) {
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
        mapbox::feature::feature<std::int16_t> f;
        f.geometry = std::move(ls);
        f.properties["ele"] = line.elevation;
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

void ContourTile::populateFromDEM(const RasterDEMTile& demTile) {
    HillshadeBucket* bucket = demTile.getBucket();
    if (bucket == nullptr) return;
    const DEMData& dem = bucket->getDEMData();

    // Snapshot the inner dim×dim heights on the render thread. The DEMData
    // image is shared with backfillBorder which mutates the 1-pixel border
    // when neighbour tiles arrive — we'd race if we sampled from the
    // background. Snapshotting up front (small: 512×512×2 bytes ≈ 0.5 MB at
    // worst for our pipeline) is simpler and safer than coordinating reads.
    const int dim = dem.dim;
    auto heights = std::make_shared<std::vector<std::int16_t>>();
    heights->reserve(static_cast<std::size_t>(dim) * dim);
    for (int y = 0; y < dim; y++) {
        for (int x = 0; x < dim; x++) {
            heights->push_back(static_cast<std::int16_t>(dem.get(x, y)));
        }
    }

    // Dispatch marching-squares to a worker; reply on the render thread and
    // call setData. WeakPtr guards against the tile being destroyed (panned
    // away) before the reply lands.
    Scheduler::GetBackground()->scheduleAndReplyValue(
        util::SimpleIdentity::Empty,
        [heights, dim]() {
            // Hard-coded interval for now — Task 5 plumbs through
            // ContourSource's IntervalSchedule per zoom.
            algorithm::contour::ContourThresholds thresholds;
            thresholds.interval = 100.0;
            thresholds.extent = util::EXTENT;
            thresholds.buffer = 0;
            const auto lines = algorithm::contour::generateContours(*heights, dim, dim, thresholds);
            return toFeatures(lines);
        },
        [self = weakFactory.makeWeakPtr(), this](mapbox::feature::feature_collection<std::int16_t> features) {
            if (auto guard = self.lock(); self) {
                setData(std::make_unique<GeoJSONTileData>(std::move(features)));
            }
        });
}

} // namespace mbgl
