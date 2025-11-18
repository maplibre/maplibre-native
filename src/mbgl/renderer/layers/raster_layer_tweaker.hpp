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
    RasterLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~RasterLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer = nullptr;

#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
#endif
};

} // namespace mbgl
