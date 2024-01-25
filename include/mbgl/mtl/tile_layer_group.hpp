#pragma once

#include <mbgl/renderer/layer_group.hpp>

namespace mbgl {

class PaintParameters;

namespace mtl {

/**
 A layer group for tile-based drawables
 */
class TileLayerGroup : public mbgl::TileLayerGroup {
public:
    TileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~TileLayerGroup() override {}

    void upload(gfx::UploadPass&) override;
    void render(RenderOrchestrator&, PaintParameters&) override;

protected:
};

} // namespace mtl
} // namespace mbgl
