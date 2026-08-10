#pragma once

#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/style/sources/custom_vector_source.hpp>
#include <mbgl/actor/mailbox.hpp>

namespace mbgl {

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

} // namespace mbgl
