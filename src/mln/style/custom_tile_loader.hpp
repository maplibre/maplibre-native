#pragma once

#include <mln/actor/actor_ref.hpp>
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/util/geojson.hpp>

#include <map>
#include <mutex>

namespace mln {

class CustomGeometryTile;

namespace style {

class CustomTileLoader {
public:
    CustomTileLoader(const CustomTileLoader&) = delete;
    CustomTileLoader& operator=(const CustomTileLoader&) = delete;

    using OverscaledIDFunctionTuple = std::tuple<uint8_t, int16_t, ActorRef<CustomGeometryTile>>;

    CustomTileLoader(const TileFunction& fetchTileFn, const TileFunction& cancelTileFn);

    void fetchTile(const OverscaledTileID& tileID, const ActorRef<CustomGeometryTile>& tileRef);
    void cancelTile(const OverscaledTileID& tileID);

    void removeTile(const OverscaledTileID& tileID);
    void setTileData(const CanonicalTileID& tileID, const GeoJSON& data);

    void invalidateTile(const CanonicalTileID&);
    void invalidateRegion(const LatLngBounds&, Range<uint8_t>);

private:
    void invokeTileFetch(const CanonicalTileID& tileID);
    void invokeTileCancel(const CanonicalTileID& tileID);

    TileFunction fetchTileFunction;
    TileFunction cancelTileFunction;
    std::unordered_map<CanonicalTileID, std::vector<OverscaledIDFunctionTuple>> tileCallbackMap;
    // Keep around a cache of tile data to serve back for wrapped and over-zooomed tiles
    std::map<CanonicalTileID, std::unique_ptr<GeoJSON>> dataCache;
    std::mutex dataMutex;
};

} // namespace style
} // namespace mln
