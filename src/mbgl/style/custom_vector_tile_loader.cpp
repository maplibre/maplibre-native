#include <mbgl/style/custom_vector_tile_loader.hpp>
#include <mbgl/tile/custom_vector_tile.hpp>

namespace mbgl {
namespace style {

CustomVectorTileLoader::CustomVectorTileLoader(const TileFunction& fetchTileFn, const TileFunction& cancelTileFn)
    : fetchTileFunction(fetchTileFn),
      cancelTileFunction(cancelTileFn) {}

void CustomVectorTileLoader::fetchTile(const OverscaledTileID& tileID, const ActorRef<CustomVectorTile>& tileRef) {
    std::scoped_lock guard(dataMutex);
    auto cachedTileData = dataCache.find(tileID.canonical);
    if (cachedTileData != dataCache.end()) {
        tileRef.invoke(&CustomVectorTile::setTileData, cachedTileData->second.first, cachedTileData->second.second);
    }
    auto tileCallbacks = tileCallbackMap.find(tileID.canonical);
    if (tileCallbacks == tileCallbackMap.end()) {
        auto tuple = std::make_tuple(tileID.overscaledZ, tileID.wrap, tileRef);
        tileCallbackMap.insert({tileID.canonical, std::vector<OverscaledIDFunctionTuple>(1, tuple)});
    } else {
        for (auto& iter : tileCallbacks->second) {
            if (std::get<0>(iter) == tileID.overscaledZ && std::get<1>(iter) == tileID.wrap) {
                std::get<2>(iter) = tileRef;
                return;
            }
        }
        tileCallbacks->second.emplace_back(std::make_tuple(tileID.overscaledZ, tileID.wrap, tileRef));
    }
    if (cachedTileData == dataCache.end()) {
        invokeTileFetch(tileID.canonical);
    }
}

void CustomVectorTileLoader::cancelTile(const OverscaledTileID& tileID) {
    std::scoped_lock guard(dataMutex);
    if (tileCallbackMap.contains(tileID.canonical)) {
        invokeTileCancel(tileID.canonical);
        // Erase so a subsequent fetchTile for the same tile re-issues the fetch.
        // Intentionally diverges from CustomTileLoader which omits this — that is a
        // latent bug there that needs a separate cleanup.
        tileCallbackMap.erase(tileID.canonical);
    }
}

void CustomVectorTileLoader::removeTile(const OverscaledTileID& tileID) {
    std::scoped_lock guard(dataMutex);
    auto tileCallbacks = tileCallbackMap.find(tileID.canonical);
    if (tileCallbacks == tileCallbackMap.end()) return;
    for (auto iter = tileCallbacks->second.begin(); iter != tileCallbacks->second.end(); iter++) {
        if (std::get<0>(*iter) == tileID.overscaledZ && std::get<1>(*iter) == tileID.wrap) {
            tileCallbacks->second.erase(iter);
            invokeTileCancel(tileID.canonical);
            break;
        }
    }
    if (tileCallbacks->second.empty()) {
        tileCallbackMap.erase(tileCallbacks);
        dataCache.erase(tileID.canonical);
    }
}

void CustomVectorTileLoader::setTileData(const CanonicalTileID& tileID,
                                         const std::shared_ptr<const std::string>& data,
                                         TileDataFormat format) {
    std::scoped_lock guard(dataMutex);
    auto iter = tileCallbackMap.find(tileID);
    if (iter == tileCallbackMap.end()) return;
    for (auto& tuple : iter->second) {
        auto actor = std::get<2>(tuple);
        actor.invoke(&CustomVectorTile::setTileData, data, format);
    }
    dataCache[tileID] = std::make_pair(data, format);
}

void CustomVectorTileLoader::setTileError(const CanonicalTileID& tileID, std::exception_ptr error) {
    std::scoped_lock guard(dataMutex);
    auto iter = tileCallbackMap.find(tileID);
    if (iter == tileCallbackMap.end()) return;
    for (auto& tuple : iter->second) {
        auto actor = std::get<2>(tuple);
        actor.invoke(&CustomVectorTile::setTileError, error);
    }
    tileCallbackMap.erase(iter);
    dataCache.erase(tileID);
}

void CustomVectorTileLoader::invalidateTile(const CanonicalTileID& tileID) {
    std::scoped_lock guard(dataMutex);
    auto tileCallbacks = tileCallbackMap.find(tileID);
    if (tileCallbacks == tileCallbackMap.end()) return;
    for (auto& iter : tileCallbacks->second) {
        auto actor = std::get<2>(iter);
        actor.invoke(&CustomVectorTile::invalidateTileData);
        invokeTileCancel(tileID);
    }
    tileCallbackMap.erase(tileCallbacks);
    dataCache.erase(tileID);
}

void CustomVectorTileLoader::invokeTileFetch(const CanonicalTileID& tileID) {
    if (fetchTileFunction != nullptr) {
        fetchTileFunction(tileID);
    }
}

void CustomVectorTileLoader::invokeTileCancel(const CanonicalTileID& tileID) {
    if (cancelTileFunction != nullptr) {
        cancelTileFunction(tileID);
    }
}

} // namespace style
} // namespace mbgl
