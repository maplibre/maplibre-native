#pragma once

#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/gl/uniform_buffer_gl.hpp>

namespace mbgl {

class PaintParameters;

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

    const gfx::UniformBufferArray& getUniformBuffers() const override { return uniformBuffers; };

    gfx::UniformBufferArray& mutableUniformBuffers() override { return uniformBuffers; };

protected:
    UniformBufferArrayGL uniformBuffers;
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

    const gfx::UniformBufferArray& getUniformBuffers() const override { return uniformBuffers; };

    gfx::UniformBufferArray& mutableUniformBuffers() override { return uniformBuffers; };

protected:
    UniformBufferArrayGL uniformBuffers;
};

} // namespace gl
} // namespace mbgl
