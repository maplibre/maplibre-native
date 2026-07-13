#include <mbgl/renderer/query.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/tile/geojson_tile.hpp>
#include <mbgl/tile/geojson_tile_data.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <utility>

namespace mbgl {

using TileFeatures = style::GeoJSONData::TileFeatures;

GeoJSONTile::GeoJSONTile(const OverscaledTileID& overscaledTileID,
                         std::string sourceID_,
                         const TileParameters& parameters,
                         std::shared_ptr<style::GeoJSONData> data_,
                         TileObserver* observer_)
    : GeometryTile(overscaledTileID, std::move(sourceID_), parameters, observer_) {
    updateData(std::move(data_), false /*needsRelayout*/, parameters.isUpdateSynchronous);
}

void GeoJSONTile::updateData(std::shared_ptr<style::GeoJSONData> data_, bool needsRelayout, bool runSynchronously) {
    MLN_TRACE_FUNC();

    assert(data_);
    data = std::move(data_);
    if (needsRelayout) reset();
    data->getTile(
        id.canonical,
        [this, self = weakFactory.makeWeakPtr(), capturedData = data.get()](TileFeatures features) {
            // If the data has changed, a new request is being processed, ignore this one
            if (auto guard = self.lock(); self && data.get() == capturedData) {
                setData(std::make_unique<GeoJSONTileData>(std::move(features)));
            }
        },
        runSynchronously);
}

void GeoJSONTile::querySourceFeatures(std::vector<Feature>& result, const SourceQueryOptions& options) {
    MLN_TRACE_FUNC();

    // Ignore the sourceLayer, there is only one
    if (auto tileData = getData()) {
        if (auto layer = tileData->getLayer({})) {
            auto featureCount = layer->featureCount();
            for (std::size_t i = 0; i < featureCount; i++) {
                auto feature = layer->getFeature(i);

                // Apply filter, if any
                if (options.filter && !(*options.filter)(style::expression::EvaluationContext{
                                          static_cast<float>(this->id.overscaledZ), feature.get()})) {
                    continue;
                }

                result.push_back(convertFeature(*feature, id.canonical));
            }
        }
    }
}

} // namespace mbgl
