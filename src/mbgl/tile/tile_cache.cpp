#include <mbgl/tile/tile_cache.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <cassert>

namespace mbgl {

TileCache::~TileCache() {
    MLN_TRACE_FUNC();

    clear();
    pendingReleases.clear();

    std::unique_lock counterLock{deferredSignalLock};
    deferredSignal.wait(counterLock, [&]() { return deferredDeletionsPending == 0; });
}

void TileCache::setSize(size_t size_) {
    MLN_TRACE_FUNC();

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
/// This exists solely to prevent a problem where temporary lambda captures
/// are retained for the duration of the scope instead of being destroyed immediately.
struct CaptureWrapper {
    CaptureWrapper(std::vector<std::unique_ptr<Tile>>&& items_)
        : items(items_.size()) {
        std::ranges::move(items_, items.begin());
    }
    CaptureWrapper(CaptureWrapper&&) = default;

    /// This copy constructor is required to build, but doesn't seem to be called.
    CaptureWrapper(const CaptureWrapper& other)
        : items(other.items) {}

    std::vector<std::shared_ptr<Tile>> items;
};
} // namespace

void TileCache::deferredRelease(std::unique_ptr<Tile>&& tile) {
    MLN_TRACE_FUNC();

    tile->cancel();
    pendingReleases.push_back(std::move(tile));
}

void TileCache::deferPendingReleases() {
    MLN_TRACE_FUNC();

    constexpr std::size_t scheduleThreshold = 1;
    if (pendingReleases.size() < scheduleThreshold) {
        return;
    }

    // Block destruction until the cleanup task is complete
    {
        std::scoped_lock counterLock{deferredSignalLock};
        deferredDeletionsPending++;
    }

    CaptureWrapper wrap{std::move(pendingReleases)};
    pendingReleases.clear();

    // The `std::function` must be created in a separate statement from the `schedule` call.
    // Creating a `std::function` from a lambda involves a copy, which is why we must use
    // `shared_ptr` rather than `unique_ptr` for the capture.  As a result, a temporary holds
    // a reference until the construction is complete and the lambda is destroyed.
    // If this temporary outlives the `schedule` call, and the function is executed immediately
    // by a waiting thread and is already complete, that temporary reference ends up being the
    // last one and the destruction actually occurs here on this thread.
    std::function<void()> func{[tile_{CaptureWrapper{std::move(wrap)}}, this]() mutable {
        MLN_TRACE_ZONE(deferPendingReleases lambda);
        MLN_ZONE_VALUE(tile_.items.size());
        tile_.items.clear();

        std::scoped_lock counterLock(deferredSignalLock);
        deferredDeletionsPending--;
        deferredSignal.notify_all();
    }};

    threadPool.schedule(std::move(func));
}

void TileCache::add(const OverscaledTileID& key, std::unique_ptr<Tile>&& tile) {
    MLN_TRACE_FUNC();

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
