#pragma once

#include <mbgl/renderer/layer_group.hpp>

namespace mbgl {
namespace gl {

/**
 A layer group for tile-based drawables
 */
class TileLayerGroupGL : public TileLayerGroup {
public:
    TileLayerGroupGL(int32_t layerIndex, std::size_t initialCapacity);
    ~TileLayerGroupGL() override { }
    
    void upload(gfx::Context&, gfx::UploadPass&) override;
    void render(RenderOrchestrator&, PaintParameters&) override;
    
protected:
};

} // namespace gl
} // namespace mbgl

