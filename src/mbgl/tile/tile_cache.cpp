#include <mbgl/tile/tile_cache.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <cassert>

namespace mbgl {

void TileCache::setSize(size_t size_) {
    size = size_;

    while (orderedKeys.size() > size) {
        auto key = orderedKeys.front();
        orderedKeys.remove(key);
        tiles.erase(key);
    }

    assert(orderedKeys.size() <= size);
}

void TileCache::add(const OverscaledTileID& key, std::unique_ptr<Tile> tile) {
    if (!tile->isRenderable() || !size) {
        tile->cancel();
        threadPool->schedule([tile_{std::shared_ptr<Tile>(std::move(tile))}]() {});
        return;
    }

    if (tiles.find(key) != tiles.end()) {
        // remove existing tile key
        orderedKeys.remove(key);
        tile->cancel();
        threadPool->schedule([tile_{std::shared_ptr<Tile>(std::move(tile))}]() {});
    } else {
        tiles.emplace(key, std::move(tile));
    }

    // (re-)insert tile key as newest
    orderedKeys.push_back(key);

    // purge oldest key/tile if necessary
    if (orderedKeys.size() > size) {
        auto removedTile = pop(orderedKeys.front());
        removedTile->cancel();

        threadPool->schedule([tile_{std::shared_ptr<Tile>(std::move(removedTile))}]() {});
    }

    assert(orderedKeys.size() <= size);
}

Tile* TileCache::get(const OverscaledTileID& key) {
    auto it = tiles.find(key);
    if (it != tiles.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

std::unique_ptr<Tile> TileCache::pop(const OverscaledTileID& key) {
    std::unique_ptr<Tile> tile;

    auto it = tiles.find(key);
    if (it != tiles.end()) {
        tile = std::move(it->second);
        tiles.erase(it);
        orderedKeys.remove(key);
        assert(tile->isRenderable());
    }

    return tile;
}

bool TileCache::has(const OverscaledTileID& key) {
    return tiles.find(key) != tiles.end();
}

void TileCache::clear() {
    orderedKeys.clear();
    tiles.clear();
}

} // namespace mbgl
