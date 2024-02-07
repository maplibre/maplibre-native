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

namespace {
template <typename T>
struct CaptureWrapper {
    CaptureWrapper(std::unique_ptr<T>&& item_)
        : item(std::move(item_)) {}
    CaptureWrapper(const CaptureWrapper& other)
        : item(other.item) {}
    std::shared_ptr<T> item;
};
} // namespace

void TileCache::deferredRelease(std::unique_ptr<Tile>&& tile) {
    tile->cancel();

    // The `std::function` must be created in a separate statement from the `schedule` call.
    // Creating a `std::function` from a lambda involves a copy, which is why we must use
    // `shared_ptr` rather than `unique_ptr` for the capture.  As a result, a temporary holds
    // a reference until the construction is complete and the lambda is destroyed.
    // If this temporary outlives the `schedule` call, and the function is executed immediately
    // by a waiting thread and is already complete, that temporary reference ends up being the
    // last one and the destruction actually occurs here on this thread.
    std::function<void()> func{[tile_{CaptureWrapper<Tile>{std::move(tile)}}]() {
    }};
    // std::function<void()> func{[tile_{std::shared_ptr<Tile>(std::move(tile))}]() { }};

    threadPool->schedule(std::move(func));
}

void TileCache::add(const OverscaledTileID& key, std::unique_ptr<Tile>&& tile) {
    tile->renderThreadId = std::this_thread::get_id();
    if (!tile->isRenderable() || !size) {
        deferredRelease(std::move(tile));
        return;
    }

    const auto result = tiles.insert(std::make_pair(key, std::unique_ptr<Tile>{}));
    if (result.second) {
        // inserted
        result.first->second = std::move(tile);
    } else {
        // already present
        // remove existing tile key to move it to the end
        orderedKeys.remove(key);
        // release the newly-provided item
        deferredRelease(std::move(tile));
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

    const auto it = tiles.find(key);
    if (it != tiles.end()) {
        tile = std::move(tiles.extract(it).mapped());
        orderedKeys.remove(key);
        assert(tile->isRenderable());
    }

    return tile;
}

bool TileCache::has(const OverscaledTileID& key) {
    return tiles.find(key) != tiles.end();
}

void TileCache::clear() {
    for (auto& item : tiles) {
        deferredRelease(std::move(item.second));
    }
    orderedKeys.clear();
    tiles.clear();
}

} // namespace mbgl
