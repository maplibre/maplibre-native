#pragma once

#include <mbgl/tile/tile_operation.hpp>
#include <string>
#include <exception>

namespace mbgl {

class RenderSource;
class OverscaledTileID;

class RenderSourceObserver {
public:
    virtual ~RenderSourceObserver() = default;

    virtual void onTileChanged(RenderSource&, const OverscaledTileID&) {}
    virtual void onTileError(RenderSource&, const OverscaledTileID&, std::exception_ptr) {}
    virtual void onTileAction(RenderSource&, TileOperation, const OverscaledTileID&, const std::string&) {}
};

} // namespace mbgl
