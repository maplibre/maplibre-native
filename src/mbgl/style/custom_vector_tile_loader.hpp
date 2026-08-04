#pragma once

#include <mbgl/actor/actor_ref.hpp>
#include <mbgl/style/sources/custom_vector_source.hpp>
#include <mbgl/tile/tile_id.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace mbgl {

class CustomVectorTile;

namespace style {

class CustomVectorTileLoader {
public:
    CustomVectorTileLoader(const CustomVectorTileLoader&) = delete;
    CustomVectorTileLoader& operator=(const CustomVectorTileLoader&) = delete;

    using OverscaledIDFunctionTuple = std::tuple<uint8_t, int16_t, ActorRef<CustomVectorTile>>;

    CustomVectorTileLoader(const TileFunction& fetchTileFn, const TileFunction& cancelTileFn);

    void fetchTile(const OverscaledTileID& tileID, const ActorRef<CustomVectorTile>& tileRef);
    void cancelTile(const OverscaledTileID& tileID);
    void removeTile(const OverscaledTileID& tileID);

    void setTileData(const CanonicalTileID& tileID,
                     const std::shared_ptr<const std::string>& data,
                     TileDataFormat format);
    void setTileError(const CanonicalTileID& tileID, std::exception_ptr error);
    void invalidateTile(const CanonicalTileID&);

private:
    void invokeTileFetch(const CanonicalTileID& tileID);
    void invokeTileCancel(const CanonicalTileID& tileID);

    TileFunction fetchTileFunction;
    TileFunction cancelTileFunction;
    std::unordered_map<CanonicalTileID, std::vector<OverscaledIDFunctionTuple>> tileCallbackMap;
    std::map<CanonicalTileID, std::pair<std::shared_ptr<const std::string>, TileDataFormat>> dataCache;
    std::mutex dataMutex;
};

} // namespace style
} // namespace mbgl
