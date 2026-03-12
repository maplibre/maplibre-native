#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

namespace mbgl {

/**
    Color relief layer specific tweaker
 */
class ColorReliefLayerTweaker : public LayerTweaker {
public:
    ColorReliefLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

    ~ColorReliefLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
    gfx::UniformBufferPtr tilePropsUniformBuffer;
#endif
};

} // namespace mbgl
