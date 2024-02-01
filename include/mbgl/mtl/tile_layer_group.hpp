#pragma once

#include <mbgl/mtl/mtl_fwd.hpp>
#include <mbgl/renderer/layer_group.hpp>

#include <Foundation/NSSharedPtr.hpp>

#include <optional>

namespace mbgl {
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
    std::optional<MTLDepthStencilStatePtr> stateNone;
    std::optional<MTLDepthStencilStatePtr> stateDepth;
};

} // namespace mtl
} // namespace mbgl
