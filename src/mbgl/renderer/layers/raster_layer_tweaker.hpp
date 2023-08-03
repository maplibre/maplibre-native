#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

namespace gfx {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

/**
    Raster layer tweaker
 */
class RasterLayerTweaker : public LayerTweaker {
public:
    RasterLayerTweaker(std::string id, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id), properties){}

public:
    ~RasterLayerTweaker() override = default;

    void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer = nullptr;
};

} // namespace mbgl
