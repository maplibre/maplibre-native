#pragma once

#include <mln/tile/geometry_tile.hpp>
#include <mln/style/sources/custom_vector_source.hpp>
#include <mln/actor/mailbox.hpp>

namespace mln {

class TileParameters;

namespace style {
class CustomVectorTileLoader;
} // namespace style

class CustomVectorTile : public GeometryTile {
public:
    CustomVectorTile(const OverscaledTileID&,
                     std::string sourceID,
                     const TileParameters&,
                     ActorRef<style::CustomVectorTileLoader> loader,
                     TileObserver* observer = nullptr);
    ~CustomVectorTile() override;

    void setTileData(const std::shared_ptr<const std::string>& data, style::TileDataFormat format);
    void setTileError(std::exception_ptr error);
    void invalidateTileData();
    void setNecessity(TileNecessity) final;

private:
    bool stale = true;
    TileNecessity necessity;
    ActorRef<style::CustomVectorTileLoader> loader;
    std::shared_ptr<Mailbox> mailbox;
    ActorRef<CustomVectorTile> actorRef;
};

} // namespace mln
