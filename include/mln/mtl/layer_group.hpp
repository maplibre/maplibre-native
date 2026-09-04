#pragma once

#include <mln/renderer/layer_group.hpp>
#include <mln/mtl/mtl_fwd.hpp>
#include <mln/mtl/uniform_buffer.hpp>

#include <Foundation/NSSharedPtr.hpp>

#include <optional>

namespace mln {

class PaintParameters;

namespace mtl {

class RenderPass;

/**
 A layer group for non-tile-based drawables
 */
class LayerGroup : public mln::LayerGroup {
public:
    LayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name, bool renderToTerrain);
    ~LayerGroup() override {}

    void upload(gfx::UploadPass&) override;
    void render(RenderOrchestrator&, PaintParameters&) override;

    const gfx::UniformBufferArray& getUniformBuffers() const override { return uniformBuffers; };

    gfx::UniformBufferArray& mutableUniformBuffers() override { return uniformBuffers; };

protected:
    UniformBufferArray uniformBuffers;

    // Depth-only / no-op depth-stencil states for 3D drawables, cached across
    // frames like TileLayerGroup's: mtl::Drawable::draw sets no depth-stencil
    // state when a drawable is 3D (the layer group owns it, so every 3D drawable
    // in the group shares one stencilModeFor3D value), so the group must provide
    // it or the encoder default applies - depth test Always, write off.
    std::optional<MTLDepthStencilStatePtr> stateNone;
    std::optional<MTLDepthStencilStatePtr> stateDepth;
};

} // namespace mtl
} // namespace mln
