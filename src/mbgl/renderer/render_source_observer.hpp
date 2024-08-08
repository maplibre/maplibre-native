#pragma once

#include <exception>

namespace mbgl {

class RenderSource;
class OverscaledTileID;

class RenderSourceObserver {
public:
    virtual ~RenderSourceObserver() = default;

    virtual void onTileChanged(RenderSource&, const OverscaledTileID&) {}
    virtual void onTileError(RenderSource&, const OverscaledTileID&, std::exception_ptr) {}
    virtual void onTileRequested(RenderSource&, const OverscaledTileID&) {};
    virtual void onTileLoadedFromNetwork(RenderSource&, const OverscaledTileID&) {};
    virtual void onTileLoadedFromDisk(RenderSource&, const OverscaledTileID&) {};
    virtual void onTileFailedToLoad(RenderSource&, const OverscaledTileID&) {};
    virtual void onTileFinishedLoading(RenderSource&, const OverscaledTileID&) {};
};

} // namespace mbgl
