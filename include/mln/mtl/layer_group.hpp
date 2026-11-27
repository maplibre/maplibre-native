#pragma once

#include <mln/mtl/mtl_fwd.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/mtl/uniform_buffer.hpp>

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
    LayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~LayerGroup() override {}

    void upload(gfx::UploadPass&) override;
    void render(RenderOrchestrator&, PaintParameters&) override;

    const gfx::UniformBufferArray& getUniformBuffers() const override { return uniformBuffers; };

    gfx::UniformBufferArray& mutableUniformBuffers() override { return uniformBuffers; };

protected:
    std::optional<MTLDepthStencilStatePtr> stateDepthReadOnly;
    std::optional<MTLDepthStencilStatePtr> stateDepthReadWrite;
    std::optional<MTLDepthStencilStatePtr> stateNone;
    UniformBufferArray uniformBuffers;
};

} // namespace mtl
} // namespace mln
