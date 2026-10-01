#pragma once

#include <mln/mtl/mtl_fwd.hpp>
#include <mln/mtl/uniform_buffer.hpp>
#include <mln/renderer/layer_group.hpp>

#include <Foundation/NSSharedPtr.hpp>

#include <optional>

namespace mln {

class PaintParameters;

namespace mtl {

class RenderPass;

/**
 A layer group for tile-based drawables
 */
class TileLayerGroup : public mln::TileLayerGroup {
public:
    TileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~TileLayerGroup() override {}

    void upload(gfx::UploadPass&) override;
    void render(RenderOrchestrator&, PaintParameters&) override;

    const gfx::UniformBufferArray& getUniformBuffers() const override { return uniformBuffers; };

    gfx::UniformBufferArray& mutableUniformBuffers() override { return uniformBuffers; };

    void bindUniformBuffers(RenderPass&) const noexcept;
    void unbindUniformBuffers(RenderPass&) const noexcept {}

protected:
    std::optional<MTLDepthStencilStatePtr> stateNone;
    std::optional<MTLDepthStencilStatePtr> stateDepth;
    UniformBufferArray uniformBuffers;
};

} // namespace mtl
} // namespace mln
