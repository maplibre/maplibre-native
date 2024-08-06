#pragma once

#include <mbgl/tile/tile_id.hpp>
#include <exception>

namespace mbgl {

class Tile;

class TileObserver {
public:
    virtual ~TileObserver() = default;

    virtual void onTileChanged(Tile&) {}
    virtual void onTileError(Tile&, std::exception_ptr) {}
    virtual void onTileRequested(Tile&) {};
    virtual void onTileLoadedFromNetwork(Tile&) {};
    virtual void onTileLoadedFromDisk(Tile&) {};
    virtual void onTileFailedToLoad(Tile&) {};
    virtual void onTileFinishedLoading(Tile&) {};
};

} // namespace mbgl
