#include <mbgl/tile/raster_tile.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/style/source.hpp>
#include <mbgl/tile/raster_tile_worker.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>
#include <mbgl/tile/tile_observer.hpp>
#include <utility>

namespace mbgl {

RasterTile::RasterTile(const OverscaledTileID& id_, const TileParameters& parameters, const Tileset& tileset)
    : Tile(Kind::Raster, id_),
      loader(*this, id_, parameters, tileset),
      threadPool(Scheduler::GetBackground()),
      mailbox(std::make_shared<Mailbox>(*Scheduler::GetCurrent())),
      worker(Scheduler::GetBackground(), ActorRef<RasterTile>(*this, mailbox)) {}

RasterTile::~RasterTile() {
    markObsolete();

    // The bucket has resources that need to be released on the render thread.
    if (bucket) {
        threadPool->runOnRenderThread([bucket_{std::move(bucket)}]() {});
    }
}

std::unique_ptr<TileRenderData> RasterTile::createRenderData() {
    return std::make_unique<SharedBucketTileRenderData<RasterBucket>>(bucket);
}

void RasterTile::setError(std::exception_ptr err) {
    loaded = true;
    observer->onTileError(*this, std::move(err));
}

void RasterTile::setMetadata(std::optional<Timestamp> modified_, std::optional<Timestamp> expires_) {
    modified = std::move(modified_);
    expires = std::move(expires_);
}

void RasterTile::setData(const std::shared_ptr<const std::string>& data) {
    if (!obsolete) {
        pending = true;
        ++correlationID;
        worker.self().invoke(&RasterTileWorker::parse, data, correlationID);
    }
}

void RasterTile::onParsed(std::unique_ptr<RasterBucket> result, const uint64_t resultCorrelationID) {
    if (!obsolete) {
        bucket = std::move(result);
        loaded = true;
        if (resultCorrelationID == correlationID) {
            pending = false;
        }
        renderable = static_cast<bool>(bucket);
        observer->onTileChanged(*this);
    }
}

void RasterTile::onError(std::exception_ptr err, const uint64_t resultCorrelationID) {
    loaded = true;
    if (resultCorrelationID == correlationID) {
        pending = false;
    }
    observer->onTileError(*this, std::move(err));
}

bool RasterTile::layerPropertiesUpdated(const Immutable<style::LayerProperties>&) {
    return bool(bucket);
}

void RasterTile::setMask(TileMask&& mask) {
    if (bucket) {
        bucket->setMask(std::move(mask));
    }
}

void RasterTile::setNecessity(TileNecessity necessity) {
    loader.setNecessity(necessity);
}

void RasterTile::setUpdateParameters(const TileUpdateParameters& params) {
    loader.setUpdateParameters(params);
}

void RasterTile::cancel() {
    markObsolete();
}

void RasterTile::markObsolete() {
    obsolete = true;
    pending = false;
    mailbox->abandon();
}

} // namespace mbgl
