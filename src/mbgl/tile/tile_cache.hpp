#pragma once

#include <mbgl/tile/tile_id.hpp>
#include <mbgl/tile/tile.hpp>

#include <list>
#include <memory>
#include <map>

namespace mbgl {

class Scheduler;

class TileCache {
public:
    TileCache(std::shared_ptr<Scheduler> threadPool_, size_t size_ = 0)
        : threadPool(std::move(threadPool_)),
          size(size_) {}

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
    std::shared_ptr<Scheduler> threadPool;

    size_t size;
};

} // namespace mbgl
