#pragma once

#include <mln/vulkan/uniform_buffer.hpp>
#include <mln/renderer/layer_group.hpp>

#include <optional>

namespace mln {

class PaintParameters;

namespace vulkan {

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

    const gfx::UniformBufferArray& getUniformBuffers() const override { return uniformBuffers; }
    gfx::UniformBufferArray& mutableUniformBuffers() override { return uniformBuffers; }

protected:
    UniformBufferArray uniformBuffers;
};

} // namespace vulkan
} // namespace mln
