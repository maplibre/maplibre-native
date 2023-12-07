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

    void setSize(size_t);
    size_t getSize() const { return size; };
    void add(const OverscaledTileID& key, std::unique_ptr<Tile> tile);
    std::unique_ptr<Tile> pop(const OverscaledTileID& key);
    Tile* get(const OverscaledTileID& key);
    bool has(const OverscaledTileID& key);
    void clear();

private:
    std::map<OverscaledTileID, std::unique_ptr<Tile>> tiles;
    std::list<OverscaledTileID> orderedKeys;
    std::shared_ptr<Scheduler> threadPool;

    size_t size;
};

} // namespace mbgl
