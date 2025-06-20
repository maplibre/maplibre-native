#pragma once

#include <mbgl/vulkan/uniform_buffer.hpp>
#include <mbgl/renderer/layer_group.hpp>

#include <optional>

namespace mbgl {

class PaintParameters;

namespace vulkan {

class RenderPass;

/**
 A layer group for tile-based drawables
 */
class TileLayerGroup : public mbgl::TileLayerGroup {
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
} // namespace mbgl
