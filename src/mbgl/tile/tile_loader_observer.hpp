#pragma once

#include <mbgl/tile/tile_id.hpp>

namespace mbgl {

class Tile;

class TileLoaderObserver {
public:
    virtual ~TileLoaderObserver() = default;

    virtual void onTileRequested() {};
    virtual void onTileLoadedFromNetwork() {};
    virtual void onTileLoadedFromDisk() {};
    virtual void onTileFailedToLoad() {};
};

} // namespace mbgl
