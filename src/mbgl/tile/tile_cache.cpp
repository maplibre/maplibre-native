#include <mbgl/tile/tile_cache.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/unique_function.hpp>

#include <cassert>

namespace mbgl {

TileCache::~TileCache() {
    MLN_TRACE_FUNC();

    clear();
    pendingReleases.clear();

    std::unique_lock<std::mutex> counterLock(deferredSignalLock);
    while (deferredDeletionsPending != 0) {
        deferredSignal.wait(counterLock);
    }
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
// Capturing `std::vector<std::unique_ptr<Tile>>` directly produces an error related to
// copying, but this somehow avoids the same problem.  It may be possible to eliminate it.
struct CaptureWrapper {
    std::vector<std::unique_ptr<Tile>> releases;

    CaptureWrapper(std::vector<std::unique_ptr<Tile>>&& x)
        : releases(std::move(x)) {}
    CaptureWrapper(CaptureWrapper&&) = default;
    CaptureWrapper& operator=(CaptureWrapper&&) = default;
    CaptureWrapper(CaptureWrapper const&) = delete;
    CaptureWrapper& operator=(CaptureWrapper const&) = delete;
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
        std::lock_guard counterLock{deferredSignalLock};
        deferredDeletionsPending++;
    }

    threadPool.schedule(
        util::unique_function<void()>{[this, wrap_{CaptureWrapper{std::move(pendingReleases)}}]() mutable {
            MLN_TRACE_ZONE(deferPendingReleases lambda);
            MLN_ZONE_VALUE(wrap_.releases.size());

            // Run the deletions
            wrap_.releases.clear();

            // Wake up a waiting destructor
            std::lock_guard counterLock{deferredSignalLock};
            deferredDeletionsPending--;
            deferredSignal.notify_all();
        }});
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
