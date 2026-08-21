#pragma once

#include <mln/tile/tile.hpp>
#include <mln/tile/tile_loader.hpp>
#include <mln/tile/raster_tile_worker.hpp>
#include <mln/actor/actor.hpp>

namespace mln {

class Tileset;
class TileParameters;
class RasterBucket;

namespace style {
class Layer;
} // namespace style

class RasterTile final : public Tile {
public:
    RasterTile(
        const OverscaledTileID&, std::string, const TileParameters&, const Tileset&, TileObserver* observer = nullptr);
    ~RasterTile() override;

    std::unique_ptr<TileRenderData> createRenderData() override;
    void setNecessity(TileNecessity) override;
    void setUpdateParameters(const TileUpdateParameters&) override;

    void setError(std::exception_ptr);
    void setMetadata(std::optional<Timestamp> modified, std::optional<Timestamp> expires);
    void setData(const std::shared_ptr<const std::string>& data);

    bool layerPropertiesUpdated(const Immutable<style::LayerProperties>& layerProperties) override;

    void setMask(TileMask&&) override;

    void onParsed(std::unique_ptr<RasterBucket> result, uint64_t correlationID);
    void onError(std::exception_ptr, uint64_t correlationID);

    void cancel() override;

private:
    void markObsolete();

    TileLoader<RasterTile> loader;

    TaggedScheduler threadPool;
    std::shared_ptr<Mailbox> mailbox;
    Actor<RasterTileWorker> worker;

    uint64_t correlationID = 0;

    // Contains the Bucket object for the tile. Buckets are render
    // objects and they get added by tile parsing operations.
    std::shared_ptr<RasterBucket> bucket;

    std::atomic<bool> obsolete{false};
};

} // namespace mln
