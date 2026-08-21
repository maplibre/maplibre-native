#pragma once

#include <mln/tile/geometry_tile.hpp>
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/util/feature.hpp>
#include <mln/util/geojson.hpp>
#include <mln/actor/mailbox.hpp>

namespace mln {

class TileParameters;

namespace style {
class CustomTileLoader;
} // namespace style

class CustomGeometryTile : public GeometryTile {
public:
    CustomGeometryTile(const OverscaledTileID&,
                       std::string,
                       const TileParameters&,
                       Immutable<style::CustomGeometrySource::TileOptions>,
                       ActorRef<style::CustomTileLoader> loader,
                       TileObserver* observer = nullptr);
    ~CustomGeometryTile() override;

    void setTileData(const GeoJSON& geoJSON);
    void invalidateTileData();

    void setNecessity(TileNecessity) final;

    void querySourceFeatures(std::vector<Feature>& result, const SourceQueryOptions&) override;

private:
    bool stale = true;
    std::string sourceID;
    TileNecessity necessity;
    Immutable<style::CustomGeometrySource::TileOptions> options;
    ActorRef<style::CustomTileLoader> loader;
    std::shared_ptr<Mailbox> mailbox;
    ActorRef<CustomGeometryTile> actorRef;
};

} // namespace mln
