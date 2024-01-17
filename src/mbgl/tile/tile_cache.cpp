#include <mbgl/tile/tile_cache.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <cassert>

namespace mbgl {

void TileCache::setSize(size_t size_) {
    size = size_;

    while (orderedKeys.size() > size) {
        const auto key = orderedKeys.front();
        orderedKeys.remove(key);

        auto hit = tiles.find(key);
        if (hit != tiles.end()) {
            auto tile = std::move(hit->second);
            tiles.erase(hit);
            deferredRelease(std::move(tile));
        }
    }

    assert(orderedKeys.size() <= size);
}

void TileCache::deferredRelease(std::unique_ptr<Tile>&& tile) {
    tile->cancel();

    // The `std::function` must be created in a separate statement from the `schedule` call.
    // Creating a `std::function` from a lambda involves a copy, which is why we must use
    // `shared_ptr` rather than `unique_ptr` for the capture.  As a result, a temporary holds
    // a reference until the construction is complete and the lambda is destroyed.
    // If this temporary outlives the `schedule` call, and the function is executed immediately
    // by a waiting thread and is already complete, that temporary reference ends up being the
    // last one and the destruction actually occurs here on this thread.
    std::function<void()> f{[tile_{std::shared_ptr<Tile>(std::move(tile))}]() {
    }};
    threadPool->schedule(std::move(f));
}

void TileCache::add(const OverscaledTileID& key, std::unique_ptr<Tile>&& tile) {
    if (!tile->isRenderable() || !size) {
        deferredRelease(std::move(tile));
        return;
    }

    if (tiles.find(key) != tiles.end()) {
        // remove existing tile key
        orderedKeys.remove(key);

        deferredRelease(std::move(tile));
    } else {
        tiles.emplace(key, std::move(tile));
    }

    // (re-)insert tile key as newest
    orderedKeys.push_back(key);

    // purge oldest key/tile if necessary
    if (orderedKeys.size() > size) {
        deferredRelease(pop(orderedKeys.front()));
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
