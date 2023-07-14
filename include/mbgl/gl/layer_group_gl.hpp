#pragma once

#include <mbgl/renderer/layer_group.hpp>

namespace mbgl {
namespace gl {

/**
 A layer group for tile-based drawables
 */
class TileLayerGroupGL : public TileLayerGroup {
public:
    TileLayerGroupGL(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~TileLayerGroupGL() override {}

    void upload(gfx::UploadPass&) override;
    void render(RenderOrchestrator&, PaintParameters&) override;

protected:
};

/**
 A layer group for non-tile-based drawables
 */
class LayerGroupGL : public LayerGroup {
public:
    LayerGroupGL(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~LayerGroupGL() override {}

    void upload(gfx::UploadPass&) override;
    void render(RenderOrchestrator&, PaintParameters&) override;

protected:
};

} // namespace gl
} // namespace mbgl
