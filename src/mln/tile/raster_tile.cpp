#include <mln/tile/raster_tile.hpp>

#include <mln/actor/scheduler.hpp>
#include <mln/renderer/buckets/raster_bucket.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/renderer/tile_render_data.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/response.hpp>
#include <mln/style/source.hpp>
#include <mln/tile/raster_tile_worker.hpp>
#include <mln/tile/tile_loader_impl.hpp>
#include <mln/tile/tile_observer.hpp>
#include <utility>

namespace mln {

RasterTile::RasterTile(const OverscaledTileID& id_,
                       std::string sourceID_,
                       const TileParameters& parameters,
                       const Tileset& tileset,
                       TileObserver* observer_)
    : Tile(Kind::Raster, id_, std::move(sourceID_), observer_),
      loader(*this, id_, parameters, tileset),
      threadPool(parameters.threadPool),
      mailbox(std::make_shared<Mailbox>(*Scheduler::GetCurrent())),
      worker(parameters.threadPool, ActorRef<RasterTile>(*this, mailbox)) {}

RasterTile::~RasterTile() {
    markObsolete();

    if (pending) {
        // This tile never finished loading or was abandoned, emit a cancellation event
        observer->onTileAction(id, sourceID, TileOperation::Cancelled);
    }

    // The bucket has resources that need to be released on the render thread.
    if (bucket) {
        threadPool.runOnRenderThread([bucket_{std::move(bucket)}]() {});
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
        ++correlationID;

        if (!pending) {
            observer->onTileAction(id, sourceID, TileOperation::StartParse);
        }

        pending = true;
        worker.self().invoke(&RasterTileWorker::parse, data, correlationID);
    }
}

void RasterTile::onParsed(std::unique_ptr<RasterBucket> result, const uint64_t resultCorrelationID) {
    if (!obsolete) {
        bucket = std::move(result);
        loaded = true;
        if (resultCorrelationID == correlationID) {
            pending = false;
            observer->onTileAction(id, sourceID, TileOperation::EndParse);
        }
        renderable = static_cast<bool>(bucket);
        observer->onTileChanged(*this);
    }
}

void RasterTile::onError(std::exception_ptr err, const uint64_t resultCorrelationID) {
    loaded = true;
    if (resultCorrelationID == correlationID) {
        pending = false;
        observer->onTileAction(id, sourceID, TileOperation::Error);
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
    if (pending) {
        observer->onTileAction(id, sourceID, TileOperation::Cancelled);
    }
    pending = false;
    mailbox->abandon();
}

} // namespace mln
