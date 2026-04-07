#pragma once

namespace mbgl {
namespace style {

class Terrain;

class TerrainObserver {
public:
    virtual ~TerrainObserver() = default;

    virtual void onTerrainChanged(const Terrain&) {}
};

} // namespace style
} // namespace mbgl
