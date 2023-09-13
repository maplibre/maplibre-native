#pragma once

#include <mbgl/renderer/layer_group.hpp>

namespace mbgl {
namespace mtl {

/**
 A layer group for non-tile-based drawables
 */
class LayerGroup : public mbgl::LayerGroup {
public:
    LayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~LayerGroup() override {}

    void upload(gfx::UploadPass&) override;
    void render(RenderOrchestrator&, PaintParameters&) override;

protected:
};

} // namespace mtl
} // namespace mbgl
