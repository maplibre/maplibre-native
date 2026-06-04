#include <mbgl/tile/custom_vector_tile.hpp>
#include <mbgl/tile/vector_mvt_tile_data.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/style/custom_vector_tile_loader.hpp>
#include <mbgl/tile/tile_observer.hpp>

#include <utility>

namespace mbgl {

CustomVectorTile::CustomVectorTile(const OverscaledTileID& overscaledTileID,
                                   std::string sourceID_,
                                   const TileParameters& parameters,
                                   ActorRef<style::CustomVectorTileLoader> loader_,
                                   TileObserver* observer_)
    : GeometryTile(overscaledTileID, std::move(sourceID_), parameters, observer_),
      necessity(TileNecessity::Optional),
      loader(std::move(loader_)),
      mailbox(std::make_shared<Mailbox>(*Scheduler::GetCurrent())),
      actorRef(*this, mailbox) {}

CustomVectorTile::~CustomVectorTile() {
    loader.invoke(&style::CustomVectorTileLoader::removeTile, id);
}

void CustomVectorTile::setTileData(const std::shared_ptr<const std::string>& data, style::TileDataFormat format) {
    if (obsolete) return;

    switch (format) {
        case style::TileDataFormat::MVT:
            GeometryTile::setData(data ? std::make_unique<VectorMVTTileData>(data) : nullptr);
            break;
    }
}

void CustomVectorTile::setTileError(std::exception_ptr error) {
    if (obsolete) return;
    observer->onTileError(*this, error);
}

void CustomVectorTile::invalidateTileData() {
    stale = true;
    observer->onTileChanged(*this);
}

void CustomVectorTile::setNecessity(TileNecessity newNecessity) {
    if (newNecessity != necessity || stale) {
        necessity = newNecessity;
        if (necessity == TileNecessity::Required) {
            loader.invoke(&style::CustomVectorTileLoader::fetchTile, id, actorRef);
            stale = false;
        } else if (!isRenderable()) {
            loader.invoke(&style::CustomVectorTileLoader::cancelTile, id);
        }
    }
}

} // namespace mbgl
