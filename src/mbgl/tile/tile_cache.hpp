#pragma once

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/tile/tile.hpp>

#include <list>
#include <memory>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace mbgl {

class TileCache {
public:
    TileCache(const TaggedScheduler& threadPool_, size_t size_ = 0)
        : threadPool(threadPool_),
          size(size_) {}

    ~TileCache() {
        clear();

        std::unique_lock<std::mutex> counterLock(deferredSignalLock);
        while (deferredDeletionsPending != 0) {
            deferredSignal.wait(counterLock);
        }
    }

    /// Change the maximum size of the cache.
    void setSize(size_t);

    /// Get the maximum size
    size_t getMaxSize() const { return size; }

    /// Add a new tile with the given ID.
    /// If a tile with the same ID is already present, it will be retained and the new one will be discarded.
    void add(const OverscaledTileID& key, std::unique_ptr<Tile>&& tile);

    std::unique_ptr<Tile> pop(const OverscaledTileID& key);
    Tile* get(const OverscaledTileID& key);
    bool has(const OverscaledTileID& key);
    void clear();

    /// Destroy a tile without blocking
    void deferredRelease(std::unique_ptr<Tile>&&);

private:
    std::map<OverscaledTileID, std::unique_ptr<Tile>> tiles;
    std::list<OverscaledTileID> orderedKeys;
    TaggedScheduler threadPool;
    std::atomic<size_t> deferredDeletionsPending{0};
    std::mutex deferredSignalLock;
    std::condition_variable deferredSignal;

    size_t size;
};

} // namespace mbgl
