#pragma once

#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/mtl/uniform_buffer.hpp>

namespace mbgl {

class PaintParameters;

namespace mtl {

class RenderPass;

/**
 A layer group for non-tile-based drawables
 */
class LayerGroup : public mbgl::LayerGroup {
public:
    LayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~LayerGroup() override {}

    void upload(gfx::UploadPass&) override;
    void render(RenderOrchestrator&, PaintParameters&) override;

    const gfx::UniformBufferArray& getUniformBuffers() const override { return uniformBuffers; };

    gfx::UniformBufferArray& mutableUniformBuffers() override { return uniformBuffers; };

protected:
    UniformBufferArray uniformBuffers;
};

} // namespace mtl
} // namespace mbgl
